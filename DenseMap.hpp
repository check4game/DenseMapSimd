#ifndef __DENSEMAPSIMD_H__
#define __DENSEMAPSIMD_H__

#include <stdint.h>
#include <immintrin.h>
#include <array>
#include <memory>
#include <string>
#include <span>    

namespace MZ
{
    static const uint32_t DenseBuild = 1017;

#if defined(DENSE_FIX1)
    static const std::string DenseOptimization = "fix1";
#elif defined(DENSE_FIX2)
    static const std::string DenseOptimization = "fix2";
#else
    static const std::string DenseOptimization = "optimal";
#endif

    static __forceinline uint64_t ResetLowestSetBit(uint64_t value)
    {
        return _blsr_u64(value);
    }

    static __forceinline uint64_t TrailingZeroCount(uint64_t value)
    {
        return _tzcnt_u64(value);
    }

    static __forceinline uint64_t ToIndex(uint64_t key)
    {
        return static_cast<uint64_t>((key ^ (key >> 32)) * UINT64_C(11400714819323198485));
    }

    static __forceinline uint64_t ToIndex(uint32_t key)
    {
        return static_cast<uint64_t>(key * UINT64_C(11400714819323198485));
    }

    static __forceinline uint32_t RoundUpToPowerOf2(uint32_t value)
    {
        return static_cast<uint32_t>(UINT64_C(0x1'0000'0000) >> __lzcnt(value - 1));
    }

    static const int8_t EMPTY_VALUE = static_cast<int8_t>(0x80);

    static const auto EMPTY_VECTOR = _mm_set1_epi8(EMPTY_VALUE);

    static const int8_t DIRTY_VALUE = static_cast<int8_t>(0x81);

    static const auto DIRTY_VECTOR = _mm_set1_epi8(DIRTY_VALUE);

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
        static const int32_t MIN_SIZE = 0x400; // 1024
        static const int32_t MAX_SIZE = 0x40000000; // 0x40000000 1'073'741'824

        void Clear() 
        {
            _Count = 0;

            std::span span(_controls, _Capacity);
            std::fill(span.begin(), span.end(), static_cast<int8_t>(EMPTY_VALUE));

        #if defined(DENSE_DEBUG)
            PROBE_COUNTER = CMP_COUNTER = 0;
        #endif

        }

        int32_t Count() const { return static_cast<int32_t>(_Count); }

        int32_t Capacity() const { return static_cast<int32_t>(_Capacity);  }

    #if defined(DENSE_DEBUG)

        uint64_t PROBE_COUNTER, CMP_COUNTER;

        void SetupDirtyEntries()
        {
            uint8_t array[sizeof(TEntry)];

            for (uint32_t i = 0; i < sizeof(TEntry); i++)
            {
                array[i] = static_cast<uint8_t>(i + 1);
            }

            for (uint32_t i = 0; i < _Capacity; i++)
            {
                _entries[i] = *reinterpret_cast<TEntry*>(array);
            }
        }

        std::span<TKey> GetKeysForCmp()
        {
            auto ptr = (TKey*)_entries;

            for (uint32_t i = 0; i < _Capacity; i++)
            {
                if (_controls[i] <= DIRTY_VALUE) continue;

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

            _controlsPtr.reset(_controls = new int8_t[_Capacity + VECTOR_SIZE]);

            _entriesPtr.reset(_entries = new TEntry[_Capacity + VECTOR_SIZE]);

            _MaxProbe = (2 * _Capacity - (VECTOR_SIZE * VECTOR_SIZE)) / VECTOR_SIZE;

            _CapacityMask = _Capacity - 1;

            _Capacity += VECTOR_SIZE;

            Clear();
        }

        static const auto VECTOR_SIZE = static_cast<uint8_t>(16);

        static const __forceinline int FindFirstEmptyOrDirty(uint64_t* vector16)
        {
            if ((vector16[0] >> 7) & 1) return 0;
            if ((vector16[0] >> 15) & 1) return 1;
            if ((vector16[0] >> 23) & 1) return 2;
            if ((vector16[0] >> 31) & 1) return 3;
            if ((vector16[0] >> 39) & 1) return 4;
            if ((vector16[0] >> 47) & 1) return 5;
            if ((vector16[0] >> 55) & 1) return 6;
            if ((vector16[0] >> 63) & 1) return 7;

            if ((vector16[1] >> 7) & 1) return 8;
            if ((vector16[1] >> 15) & 1) return 9;
            if ((vector16[1] >> 23) & 1) return 10;
            if ((vector16[1] >> 31) & 1) return 11;
            if ((vector16[1] >> 39) & 1) return 12;
            if ((vector16[1] >> 47) & 1) return 13;
            if ((vector16[1] >> 55) & 1) return 14;
            if ((vector16[1] >> 63) & 1) return 15;

            return -1;
        }

        /// <summary>
        /// Retrieves the 7 lowest bits from a index
        /// </summary>
        /// <param name="index">index aka HashCode</param>
        /// <returns>The 7 lowest bits of the index, [0...127]</returns>
        static const __forceinline char IndexToHashCode(uint64_t index)
        {
            return static_cast<char>(index >> 57);
        }

        uint32_t _Capacity, _CapacityMask, _MaxProbe, _Count;

        int8_t* _controls = nullptr;
        std::unique_ptr<int8_t> _controlsPtr;

        TEntry* _entries = nullptr;
        std::unique_ptr<TEntry> _entriesPtr;
    };

    template <typename TKey, typename TValue>
    class DenseHashMap : public DenseMap<TKey, TValue, KeyValueEntry<TKey, TValue>>
    {
    public:
        DenseHashMap(int32_t size) : DenseMap<TKey, TValue, KeyValueEntry<TKey, TValue>>(size) {}

        bool __forceinline Add(TKey key, TValue value);
        void __forceinline AddUnique(TKey key, TValue value);
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
        void __forceinline AddUnique(TKey key);
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
        void __forceinline AddUnique(TKey key);
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