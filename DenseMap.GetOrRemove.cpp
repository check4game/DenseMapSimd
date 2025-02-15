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

        #if defined(DENSE_HASHINDEX) || defined(DENSE_TRY_GET)
            const auto& entry = __super::_entries[index];

            #if defined(DENSE_HASHINDEX)
            if (key == entry.key) return entry.value | KEY_EXIST_MASK;
            #else
            if (key == entry.key)
            {
                value = entry.value; return true;
            }
            #endif
        #else
            #if defined(DENSE_HASHTABLE)
            if (key == __super::_entries[index])
            #else
            if (key == __super::_entries[index].key)
            #endif
            {
            #if defined(DENSE_REMOVE)
                __super::_controls[index] = DIRTY_VALUE; __super::_Count--;
            #endif
                return true;
            }
        #endif

            resultMask = ResetLowestSetBit(resultMask);
        }
    #endif

        while (resultMask != 0)
        {
        #if defined(DENSE_DEBUG)
            __super::CMP_COUNTER++;
        #endif

        #if defined(DENSE_HASHINDEX) || defined(DENSE_TRY_GET)
            const auto& entry = __super::_entries[index + TrailingZeroCount(resultMask)];

            if (key == entry.key)
            {
                #if defined(DENSE_HASHINDEX)
                return entry.value | KEY_EXIST_MASK;
                #else
                value = entry.value; return true;
                #endif
            }

        #elif defined(DENSE_REMOVE)
            auto pos = TrailingZeroCount(resultMask);
            #if defined(DENSE_HASHTABLE)
            if (key == __super::_entries[index + pos])
            #else
            if (key == __super::_entries[index + pos].key)
            #endif
            {
                __super::_controls[index + pos] = DIRTY_VALUE; __super::_Count--; return true;
            }
        #elif defined(DENSE_HASHTABLE)
            if (key == __super::_entries[index + TrailingZeroCount(resultMask)]) return true;
        #else
            if (key == __super::_entries[index + TrailingZeroCount(resultMask)].key) return true;
        #endif

            resultMask = ResetLowestSetBit(resultMask);
        }

        #if defined(DENSE_HASHINDEX)
            if (_mm_movemask_epi8(_mm_cmpeq_epi8(source, EMPTY_VECTOR))) return {};
        #else
            if (_mm_movemask_epi8(_mm_cmpeq_epi8(source, EMPTY_VECTOR))) return false;
        #endif

        #if defined(DENSE_DEBUG)
        __super::PROBE_COUNTER++;
        #endif

        jumpDistance += __super::VECTOR_SIZE; // Increase the jump distance by 16 to probe the next cluster.
        index += jumpDistance; // Move the index forward by the jump distance.           
    }

    return false;
}
