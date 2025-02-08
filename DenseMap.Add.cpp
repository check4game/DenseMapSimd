#include "DenseMap.hpp"

template <DENSE_TEMPLATE>
DENSE_FUNCTION
{
    uint64_t index = ToIndex(key);

    const int8_t hashCode = __super::IndexToHashCode(index);

    // Initialize the probing jump distance to zero, which will increase with each probe iteration.
    uint8_t jumpDistance = static_cast<uint8_t>(0);

    const auto target = _mm_set1_epi8(hashCode);

    for (uint32_t i = 0; i < __super::_MaxProbe; i++)
    {
        index &= __super::_CapacityMask;

        const auto source = _mm_loadu_si128((const __m128i*)(__super::_controls + index));

        auto resultMask = static_cast<uint64_t>(_mm_movemask_epi8(_mm_cmpeq_epi8(source, target)));

    #if defined(DENSE_FIX1) || defined(DENSE_FIX2)

        if (resultMask & 1)
        {
        #if defined(DENSE_DEBUG)
            __super::CMP_COUNTER++;
        #endif

        #if defined(DENSE_UPDATE)
            auto& entry = __super::_entries[index];

            if (key == entry.key)
            {
                entry.value = value; return false;
            }
        #elif defined(DENSE_HASHINDEX)
            const auto& entry = __super::_entries[index];
            if (key == entry.key) return entry.value | KEY_EXIST_MASK;
        #else
            if (key == __super::_entries[index].key) return false;
        #endif

            resultMask = ResetLowestSetBit(resultMask);
        }

    #endif

        while (resultMask != 0)
        {

        #if defined(DENSE_DEBUG)
            __super::CMP_COUNTER++;
        #endif

        #if defined(DENSE_UPDATE)
            auto& entry = __super::_entries[index + TrailingZeroCount(resultMask)];

            if (key == entry.key)
            {
                entry.value = value; return false;
            }
        #elif defined(DENSE_HASHINDEX)
            const auto& entry = __super::_entries[index + TrailingZeroCount(resultMask)];

            if (key == entry.key) return entry.value | KEY_EXIST_MASK;
        #else
            if (key == __super::_entries[index + TrailingZeroCount(resultMask)].key) return false;
        #endif

            resultMask = ResetLowestSetBit(resultMask);
        }

        uint64_t emptyMask = static_cast<uint64_t>(_mm_movemask_epi8(_mm_cmpeq_epi8(source, EMPTY_VECTOR)));
        emptyMask |= static_cast<uint64_t>(_mm_movemask_epi8(_mm_cmpeq_epi8(source, DIRTY_VECTOR)));
        
        if (emptyMask != 0)
        {
        #if defined(DENSE_FIX1)
            if (!(emptyMask & 1)) // if bit is one, TZC() return 0, fix for 'old' cpu
            {
                index += TrailingZeroCount(emptyMask);
            }
        #elif defined(DENSE_FIX2)
            // fix for 'old' cpu
            index += __super::FindFirstEmptyOrDirty(reinterpret_cast<uint64_t*>(__super::_controls + index));
        #else
            index += TrailingZeroCount(emptyMask);
        #endif

            __super::_controls[index] = hashCode;

        #if defined(DENSE_HASHTABLE)
            __super::_entries[index].key = key;
        #else
            auto& entry = __super::_entries[index];

            entry.key = key;

            #if defined(DENSE_HASHINDEX)
            entry.value = __super::_Count;
            #else
            entry.value = value;
            #endif

        #endif
            __super::_Count++;

        #if defined(DENSE_HASHINDEX)
            return entry.value;
        #else
            return true;
        #endif
        }

        #if defined(DENSE_DEBUG)
        __super::PROBE_COUNTER++;
        #endif

        jumpDistance += __super::VECTOR_SIZE; // Increase the jump distance by 16 to probe the next cluster.
        index += jumpDistance; // Move the index forward by the jump distance.           
    }

    return false;
}
