#include "DenseMap.hpp"

template <DENSE_TEMPLATE>
DENSE_FUNCTION
{
    auto index = ToIndex(key);

    const auto hashCode = IndexToHashCode(index);

    // Initialize the probing jump distance to zero, which will increase with each probe iteration.
    uint8_t jumpDistance = 0;

    const auto target = _mm_set1_epi8(hashCode);

    for (uint32_t i = 0; i < __super::_MaxProbe; i++)
    {
        index &= __super::_CapacityMask;

        const auto source = _mm_loadu_si128((const __m128i*)(__super::_controls + index));

        auto resultMask = (uint64_t)_mm_movemask_epi8(_mm_cmpeq_epi8(source, target));

    #if defined(DENSE_FIX1) || defined(DENSE_FIX2)
        if ((resultMask & 1) == 1)
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
            if (key == __super::_entries[index].key)
            {
            #if defined(DENSE_REMOVE)
                __super::_controls[index] = 0; __super::_Count--;
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

            if (key == __super::_entries[index + pos].key)
            {
                __super::_controls[index + pos] = 0; __super::_Count--; return true;
            }
        #else    
            if (key == __super::_entries[index + TrailingZeroCount(resultMask)].key) return true;
        #endif

            resultMask = ResetLowestSetBit(resultMask);
        }

        #if defined(DENSE_HASHINDEX)
            if (0 != _mm_movemask_epi8(_mm_cmpeq_epi8(source, _mm_setzero_si128()))) return {};
        #else
            if (0 != _mm_movemask_epi8(_mm_cmpeq_epi8(source, _mm_setzero_si128()))) return false;
        #endif

        #if defined(DENSE_DEBUG)
        __super::PROBE_COUNTER++;
        #endif

        jumpDistance += 16; // Increase the jump distance by 16 to probe the next cluster.
        index += jumpDistance; // Move the index forward by the jump distance.           
    }

    return false;
}
