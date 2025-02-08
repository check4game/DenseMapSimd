#include "DenseMap.hpp"

#include <iostream>
#include <immintrin.h>
#include <vector>
#include <iomanip>
#include <locale>
#include <chrono>
#include <span>

#include "includes/primesieve.hpp"
#include "includes/cppcrc.h"
#include "includes/xxhash.hpp"

using namespace std::chrono;

extern void LoadRandomsFromFile(const std::string& filename, std::vector<uint64_t>& data_set);
extern void GenerateRandomsToFile(const std::string& filename, uint64_t data_set_size, bool bSequential);
extern void ShowVersion(const std::string& cmd, const std::string DenseOptimization, uint32_t DenseBuild, bool bTestDataSet);

struct separate_thousands : std::numpunct<char> {
    char_type do_thousands_sep() const override { return '.'; }  // separate with commas
    string_type do_grouping() const override { return "\3"; } // groups of 3 digit
};

std::vector<uint64_t> data_set;

auto defaultLoc = std::cout.getloc();

template <typename ObjectType, typename KeyType>
void Bench(uint64_t load, ObjectType& object, bool bTestDataSet)
{
    object.Clear(); object.SetupDirtyEntries();
    
    object.Test(data_set.data(), 10'000'000);

    object.Clear(); object.SetupDirtyEntries();

    const auto t_start = std::chrono::high_resolution_clock::now();

    object.Test(data_set.data(), (int32_t)load);

    const auto t_end = std::chrono::high_resolution_clock::now();

    if (bTestDataSet)
    {
        std::cout.imbue(defaultLoc);

        auto keys = object.GetKeysForCmp();

        std::sort(keys.begin(), keys.end(), [](const KeyType a, const KeyType b) -> bool { return a < b; });

        auto seed = high_resolution_clock::now().time_since_epoch().count();

        auto crcKeys = xxh::detail3::xxhash3_impl<128>(keys.data(), keys.size(), seed);
        
        if (sizeof(KeyType) == sizeof(uint64_t))
        {
            auto spanSource = std::span<uint64_t>(data_set.data(), keys.size());
            auto spanTraget = std::span<uint64_t>((uint64_t*)keys.data(), keys.size());
            std::copy(spanSource.begin(), spanSource.end(), spanTraget.begin());
        }
        else
        {
            auto spanSource = std::span<uint64_t>(data_set.data(), keys.size());
            std::transform(spanSource.begin(), spanSource.end(), keys.begin(),
                [](uint64_t value) -> uint32_t {
                    return static_cast<uint32_t>(value); // Преобразуем uint64_t в uint32_t
                });
        }

        std::sort(keys.begin(), keys.end(), [](const KeyType a, const KeyType b) -> bool { return a < b; });

        auto crcData = xxh::detail3::xxhash3_impl<128>(keys.data(), keys.size(), seed);

        std::cout.imbue(defaultLoc);

        std::cout << std::uppercase << std::hex << std::setfill('0');

        if (crcData == crcKeys)
        {
            std::cout << "OK, ";
        }
        else
        {
            std::cout << "ER, " << seed << ", ";
        }

        std::cout << std::dec << std::setfill(' ');

        std::cout.imbue(std::locale(std::cout.getloc(), new separate_thousands));
    }

    std::cout << "time:" << std::setw(6) << std::fixed << std::setprecision(3)
        << (t_end - t_start) / 1ms / 1000.0 << std::setprecision(4) << "s, load: " << ((double)load / data_set.size())
        << ", cnt:" << std::setw(12) << object.Count()
        << ", op: " << (t_end - t_start) / 1ns / object.Count() << "ns"
#if defined(DENSE_DEBUG)
        << ", probe:" << std::setw(11) << object.PROBE_COUNTER
        << ", cmp:" << std::setw(11) << object.CMP_COUNTER
#endif
        << std::endl;
}

std::vector<std::string> cmds = 
{ "run", "rnd", "help"
};

extern std::vector<int> vector_data_set_size;

int main(int argc, char** argv)
{
    auto cmdIterator = std::find(cmds.begin(), cmds.end(), (argc > 1) ? argv[1] : "help");

    auto cmd = (cmdIterator == cmds.end()) ? "help" : *cmdIterator;

    auto tableSizeIterator = std::find(vector_data_set_size.begin(), vector_data_set_size.end(), (argc < 3) ? 128 : atoi(argv[2]));

    bool bTestDataSet = false;

    uint32_t TypeMask = 1, KeySizeMask = 0x00000000;

    bool bSequential = false;

    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-test") == 0)
        {
            bTestDataSet = true;
        }

        if (strcmp(argv[i], "-map") == 0 || strcmp(argv[i], "-hm") == 0)
        {
            TypeMask = 1;
        }

        if (strcmp(argv[i], "-hi") == 0)
        {
            TypeMask = 2;
        }

        if (strcmp(argv[i], "-ht") == 0)
        {
            TypeMask = 4;
        }

        if (strcmp(argv[i], "-all") == 0)
        {
            TypeMask = 7;
        }

        if (strcmp(argv[i], "-32") == 0)
        {
            KeySizeMask = 0x80;
        }

        if (strcmp(argv[i], "-seq") == 0)
        {
            bSequential = true;
        }
    }

    ShowVersion(cmd, MZ::DenseOptimization, MZ::DenseBuild, bTestDataSet);

    if (cmd == "help")
    {
        std::cout << std::endl;
        std::cout << "DenseMapSimd.exe run [max [min [step]]] [-32|-64] [-all|-map|-ht|-hi] [-test]" << std::endl;
        std::cout << "-hm DenseHashMap, -ht DenseHashTable, -hi DenseHashIndex" << std::endl;
        std::cout << "-32, -64 key size in bits, -64 by default" << std::endl;
        std::cout << "-test comparison dataset with a set of keys" << std::endl;

        std::cout << std::endl;

        std::cout << "DenseMapSimd.exe rnd [32|64|128|256] [-seq]" << std::endl;
        std::cout << "32|64|128|256  dataset size in MB, 128 by default" << std::endl;
        std::cout << "-seq, sequential set of numbers" << std::endl;

        std::cout << std::endl;
        return 0;
    }

    std::cout.imbue(std::locale(std::cout.getloc(), new separate_thousands));

    std::string filename = "DenseMap.64.dat";

    if (cmd == "rnd")
    {
        auto data_set_size = (vector_data_set_size.end() == tableSizeIterator) ? (128 * 1024 * 1024) : *tableSizeIterator * 1024 * 1024;

        uint8_t mode = (argc > 3 && atoi(argv[3]) >= 0 && atoi(argv[3]) < 5) ? atoi(argv[3]) : 0;

        GenerateRandomsToFile(filename, (uint64_t)data_set_size, bSequential);

        std::cout << std::endl;

        return 0;
    }
    
    LoadRandomsFromFile(filename, data_set);

    uint64_t maxLoad = data_set.size(), startLoad = 10'000'000, stepLoad = 10'000'000;

    bool bAutoStep = true;

    if (argc > 2 && argv[2][0] != '-' && atoi(argv[2]) >= 32 && atoi(argv[2]) <= (data_set.size() / 1000 / 1000))
    {
        maxLoad = atoi(argv[2]) * 1024 * 1024;
    }

    if (argc > 3 && argv[3][0] != '-' && atoi(argv[3]) >= 10 && atoi(argv[3]) <= (maxLoad / 1000 / 1000))
    {
        startLoad = (uint64_t)(atof(argv[3]) * 1000 * 1000);
    }

    if (argc > 4 && argv[4][0] != '-' && atof(argv[4]) > 0 && atof(argv[4]) <= 10)
    {
        stepLoad = (uint64_t)(atof(argv[4]) * 1000 * 1000); bAutoStep = false;
    }

    using Type01 = MZ::DenseHashMap<uint64_t, uint32_t>;
    using Type02 = MZ::DenseHashIndex<uint64_t>;
    using Type04 = MZ::DenseHashTable<uint64_t>;

    using Type81 = MZ::DenseHashMap<uint32_t, uint32_t>;
    using Type82 = MZ::DenseHashIndex<uint32_t>;
    using Type84 = MZ::DenseHashTable<uint32_t>;

    for (uint32_t iType = 1; iType <= 4; iType <<= 1)
    {
        if ((iType & TypeMask) == 0) continue;

        std::cout << std::endl;

        switch ((iType & TypeMask) | KeySizeMask)
        {
        case 0x01:
            std::cout << "-hm, DenseHashMap<uint64_t, uint32_t>::Add(key)" << std::endl;
            break;
        case 0x81:
            std::cout << "-hm, DenseHashMap<uint32_t, uint32_t>::Add(key)" << std::endl;
            break;
        case 0x02:
            std::cout << "-hi, DenseHashIndex<uint64_t>::Add(key)" << std::endl;
            break;
        case 0x82:
            std::cout << "-hi, DenseHashIndex<uint32_t>::Add(key)" << std::endl;
            break;
        case 0x04:
            std::cout << "-ht, DenseHashTable<uint64_t>::Add(key)" << std::endl;
            break;
        case 0x84:
            std::cout << "-ht, DenseHashTable<uint32_t>::Add(key)" << std::endl;
            break;
        }

        for (uint64_t load = startLoad; load < maxLoad; load += stepLoad)
        {
            switch ((iType & TypeMask) | KeySizeMask)
            {
            case 0x01:
            {
                Type01 object01((int32_t)data_set.size());
                Bench<Type01, uint64_t>(load, object01, bTestDataSet);
            }
                break;
            case 0x81:
            {
                Type81 object81((int32_t)data_set.size());
                Bench<Type81, uint32_t>(load, object81, bTestDataSet);
            }
                break;
            case 0x02:
            {
                Type02 object02((int32_t)data_set.size());
                Bench<Type02, uint64_t>(load, object02, bTestDataSet);
            }
                break;
            case 0x82:
            {
                Type82 object82((int32_t)data_set.size());
                Bench<Type82, uint32_t>(load, object82, bTestDataSet);
            }
                break;
            case 0x04:
            {
                Type04 object04((int32_t)data_set.size());
                Bench<Type04, uint64_t>(load, object04, bTestDataSet);
            }
                break;
            case 0x84:
            {
                Type84 object84((int32_t)data_set.size());
                Bench<Type84, uint32_t>(load, object84, bTestDataSet);
            }
                break;
            }

            if (bAutoStep)
            {
                
            }
        }

        std::cout << std::endl;
    }
}