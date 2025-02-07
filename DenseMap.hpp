#ifndef __DENSEMAPSIMD_H__
#define __DENSEMAPSIMD_H__

#include <stdint.h>
#include <immintrin.h>
#include <vector>
#include <memory>
#include <string>

#if defined(DENSE_DEBUG)
#include <span>    
#endif

namespace MZ
{
    static const std::string GetOptimizationType()
    {
#if defined(DENSE_FIX1)
        return "fix1";
#elif defined(DENSE_FIX2)
        return "fix2";
#else
        return "optimal";
#endif
    }

    static __forceinline uint64_t ResetLowestSetBit(uint64_t value)
    {
        return _blsr_u64(value);
    }

    static __forceinline uint64_t TrailingZeroCount(uint64_t value)
    {
        return _tzcnt_u64(value);
    }

    static __forceinline int FindFirstZero(uint64_t* vector16)
    {
        if (((vector16[0] >> 0) & 0xFF) == 0) return 0;
        if (((vector16[0] >> 8) & 0xFF) == 0) return 1;
        if (((vector16[0] >> 16) & 0xFF) == 0) return 2;
        if (((vector16[0] >> 24) & 0xFF) == 0) return 3;
        if (((vector16[0] >> 32) & 0xFF) == 0) return 4;
        if (((vector16[0] >> 40) & 0xFF) == 0) return 5;
        if (((vector16[0] >> 48) & 0xFF) == 0) return 6;
        if (((vector16[0] >> 56) & 0xFF) == 0) return 7;

        if (((vector16[1] >> 0) & 0xFF) == 0) return 8;
        if (((vector16[1] >> 8) & 0xFF) == 0) return 9;
        if (((vector16[1] >> 16) & 0xFF) == 0) return 10;
        if (((vector16[1] >> 24) & 0xFF) == 0) return 11;
        if (((vector16[1] >> 32) & 0xFF) == 0) return 12;
        if (((vector16[1] >> 40) & 0xFF) == 0) return 13;
        if (((vector16[1] >> 48) & 0xFF) == 0) return 14;
        if (((vector16[1] >> 56) & 0xFF) == 0) return 15;

        return -1;
    }

    static __forceinline uint64_t ToIndex(uint64_t key)
    {
        return static_cast<uint64_t>((key ^ (key >> 32)) * UINT64_C(11400714819323198485));
    }

    static __forceinline uint64_t ToIndex(uint32_t key)
    {
        return static_cast<uint64_t>(key * UINT64_C(11400714819323198485));
    }

    static __forceinline const char IndexToHashCode(uint64_t index)
    {
        return static_cast<char>((index >> 56) | 0x80);
    }

    static __forceinline uint32_t RoundUpToPowerOf2(uint32_t value)
    {
        return static_cast<uint32_t>(UINT64_C(0x1'0000'0000) >> __lzcnt(value - 1));
    }

#pragma pack(push, 1)

    template <typename TKey, typename TValue>
    struct KeyValueEntry
    {
        TKey key;
        TValue value;
    };

    template <typename TKey>
    struct KeyEntry
    {
        TKey key;
    };

#pragma pack(pop)

    template <typename TKey, typename TValue, typename TEntry>
    class DenseMap
    {
    public:
        static const int32_t MIN_SIZE = 1024; // 1024
        static const int32_t MAX_SIZE = 0x40000000; // 0x40000000 1'073'741'824

        void Clear() 
        {
            _Count = 0; std::fill(_controlsPtr->begin(), _controlsPtr->end(), 0);

        #if defined(DENSE_DEBUG)
            PROBE_COUNTER = CMP_COUNTER = 0;
        #endif

        }

        int32_t Count() { return (int32_t)_Count; }

        int32_t Capacity() { return (int32_t)_Capacity;  }

    #if defined(DENSE_DEBUG)

        uint64_t PROBE_COUNTER = 0, CMP_COUNTER = 0;

        virtual std::span<TKey> GetKeysForCmp()
        {
            auto ptr = (TKey*)_entries;

            for (uint32_t i = 0; i < _Capacity; i++)
            {
                if (_controls[i] == 0) continue;

                *ptr = _entries[i].key; ptr++;
            }

            return std::span<TKey>((TKey*)_entries, (ptr - (TKey*)_entries));
        }

        virtual void Test(uint64_t* data_set, int32_t load) {}

    #endif

    protected:
        DenseMap(int32_t size)
        {
            Init(size);
        }

        void Init(int32_t size)
        {
            if (size < MIN_SIZE) size = MIN_SIZE;
            if (size > MAX_SIZE) size = MAX_SIZE;

            _Capacity = RoundUpToPowerOf2(static_cast<uint32_t>(size));

            if (_Capacity > static_cast<uint32_t>(MAX_SIZE)) _Capacity = MAX_SIZE;

            _controlsPtr.reset(new std::vector<uint8_t>(_Capacity + 16));

            _entriesPtr.reset(new std::vector<TEntry>(_Capacity + 16));

            std::fill(_controlsPtr->begin(), _controlsPtr->end(), 0);

            _controls = _controlsPtr->data();

            _entries = _entriesPtr->data();

            _MaxProbe = (2 * _Capacity - 256) / 16;

            _CapacityMask = _Capacity - 1;

            _Capacity = static_cast<uint32_t>(_controlsPtr->size());

            _Count = 0;
        }

        uint32_t _Capacity, _CapacityMask, _MaxProbe, _Count;

        uint8_t* _controls = nullptr;
        std::unique_ptr<std::vector<uint8_t>> _controlsPtr;

        TEntry* _entries = nullptr;
        std::unique_ptr<std::vector<TEntry>> _entriesPtr;

        //public int EnsureCapacity(int capacity);
    };

    template <typename TKey, typename TValue>
    class DenseHashMap : public DenseMap<TKey, TValue, KeyValueEntry<TKey, TValue>>
    {
    public:
        DenseHashMap(int32_t size) : DenseMap<TKey, TValue, KeyValueEntry<TKey, TValue>>(size) {}

        bool __forceinline Add(TKey key, TValue value);
        bool __forceinline AddOrUpdate(TKey key, TValue value);
        bool __forceinline TryGetValue(TKey key, TValue& value);
        bool __forceinline Remove(TKey key);

#if defined(DENSE_DEBUG)

        void Test(uint64_t* data_set, int32_t load)
        { 
            for (int32_t i = 0; i < load; i++) Add(static_cast<TKey>(data_set[i]), static_cast<TKey>(i));
        }
#endif
    };
    
    template <typename TKey>
    class DenseHashIndex : public DenseMap<TKey, uint32_t, KeyValueEntry<TKey, uint32_t>>
    {
    public:

        static const uint32_t KEY_EXIST_MASK = 0x80000000;

        DenseHashIndex(int32_t size) : DenseMap<TKey, uint32_t, KeyValueEntry<TKey, uint32_t>>(size) {}

        uint32_t __forceinline AddOrGet(TKey key);
        uint32_t __forceinline Get(TKey key);

#if defined(DENSE_DEBUG)
        void Test(uint64_t* data_set, int32_t load)
        {
            for (int32_t i = 0; i < load; i++) AddOrGet(static_cast<TKey>(data_set[i]));
        }
#endif
    };
    
    template <typename TKey>
    class DenseHashTable : public DenseMap<TKey, uint32_t, KeyEntry<TKey>>
    {
    public:

        DenseHashTable(int32_t size) : DenseMap<TKey, uint32_t, KeyEntry<TKey>>(size) {}

        bool __forceinline Add(TKey key);
        bool __forceinline Contains(TKey key);
        bool __forceinline Remove(TKey key);

#if defined(DENSE_DEBUG)
        void Test(uint64_t* data_set, int32_t load)
        {
            for (int32_t i = 0; i < load; i++) Add(static_cast<TKey>(data_set[i]));
        }
#endif
    };
}

#endif