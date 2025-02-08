#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <algorithm>
#include <chrono>
#include <iomanip>

using namespace std::chrono;

std::vector<int> vector_data_set_size = { 32, 64, 128, 256 };

std::vector<uint64_t> buffer(128 * 1024);

#include "includes/Judy.h"

#pragma comment(lib, "libs/Judy.lib")

void LoadRandomsFromFile(const std::string& filename, std::vector<uint64_t>& data_set)
{
    std::cout << "load from '" << filename << "'";

    std::ifstream inFile;

    inFile.open(filename, std::ios_base::binary);

    if (!inFile.is_open())
    {
        std::cerr << std::endl << "Failed to open file for reading." << std::endl;
        exit(-1);
    }

    auto t_start = high_resolution_clock::now();

    uint64_t data_set_size;

    bool bSequential;

    inFile.read(reinterpret_cast<char*>(&data_set_size), sizeof(data_set_size));

    bSequential = (data_set_size == 1);

    inFile.read(reinterpret_cast<char*>(&data_set_size), sizeof(data_set_size));

    auto it = std::find(vector_data_set_size.begin(), vector_data_set_size.end(), data_set_size / 1024 / 1024);

    if (it == vector_data_set_size.end() || (data_set_size % 1024) != 0)
    {
        std::cerr << std::endl << "Failed read data, file corrupted..." << std::endl;
        exit(-1);
    }

    data_set.resize(data_set_size);

    auto bufferSize = (128LLU * 1024 / sizeof(uint64_t));

    for (uint64_t i = 0; i < data_set_size; i += bufferSize)
    {
        if (!inFile.read(reinterpret_cast<char*>(data_set.data() + i), bufferSize * sizeof(uint64_t)))
        {
            std::cerr << std::endl << "Failed read data, file corrupted..." << std::endl;
            exit(-1);
        }
    }

    inFile.close();

    auto t_end = high_resolution_clock::now();

    std::cout << ", time: " << (t_end - t_start) / 1ms / 1000.0 << " s";
    std::cout << ", size: " << data_set_size;

    if (bSequential)
    {
        std::cout << ", Sequential set of numbers";
    }
    else
    {
        std::cout << ", Unique Random set of numbers";
    }

    std::cout << std::endl;

    return;
}

void GenerateRandomsToFile(const std::string& filename, uint64_t data_set_size, bool bSequential)
{
    if (bSequential)
    {
        std::cout << "generate " << data_set_size << " Sequential set of numbers to '" << filename << "' ";
    }
    else
    {
        std::cout << "generate " << data_set_size << " Unique Random set of numbers to '" << filename << "' ";
    }

    std::ofstream outFile(filename, std::ios_base::binary | std::ios_base::trunc);

    if (!outFile.is_open()) {
        std::cerr << std::endl << "Failed to open file for writing." << std::endl;
        return;
    }

    auto t_start = high_resolution_clock::now();

    std::mt19937_64 generator(1);
    std::uniform_int_distribution<uint64_t> distribution;

    Pvoid_t JArrayMain = NULL;

    uint32_t count = 0; buffer.clear();

    uint64_t judyMask = 0x0000'0000'FFFF'FFFF;

    buffer.push_back((uint64_t)bSequential); buffer.push_back(data_set_size);

    while (count < data_set_size)
    {
        auto rnd = (bSequential) ? count : distribution(generator);

        int rc; 
        
        J1S(rc, JArrayMain, (rnd & judyMask));

        if (rc != 1) continue;

        count++; buffer.push_back(rnd);

        if (buffer.size() == buffer.capacity())
        {
            outFile.write(reinterpret_cast<const char*>(&buffer.front()), buffer.size() * sizeof(uint64_t));
            buffer.clear();
        }

        if ((count % 5'000'000) == 0)  std::cout << '.';
    }

    if (buffer.size() > 0)
    {
        outFile.write(reinterpret_cast<const char*>(&buffer.front()), buffer.size() * sizeof(uint64_t));
        buffer.clear();
    }

    outFile.flush(); outFile.close();

    auto t_end = high_resolution_clock::now();

    std::cout << std::endl << std::fixed << std::setprecision(3) << "time: " << (t_end - t_start) / 1ms / 1000.0 << " s"
        << ", size: " << count << ", MemUsed: " << Judy1MemUsed(JArrayMain) / 1024 / 1024 << " MB";

    std::cout << std::endl;

    Word_t ret; J1FA(ret, JArrayMain);
}
