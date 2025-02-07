#include "DenseMap.hpp"

#define DENSE_HASHTABLE
#define DENSE_TEMPLATE typename TKey
#define DENSE_FUNCTION bool __forceinline MZ::DenseHashTable<TKey>::Add(TKey key)
#include "DenseMap.Add.cpp"
#undef DENSE_FUNCTION
#undef DENSE_TEMPLATE
#undef DENSE_HASHTABLE


#define DENSE_TEMPLATE typename TKey
#define DENSE_FUNCTION bool __forceinline MZ::DenseHashTable<TKey>::Contains(TKey key)
#include "DenseMap.GetOrRemove.cpp"
#undef DENSE_FUNCTION
#undef DENSE_TEMPLATE

#define DENSE_REMOVE
#define DENSE_TEMPLATE typename TKey
#define DENSE_FUNCTION bool __forceinline MZ::DenseHashTable<TKey>::Remove(TKey key)
#include "DenseMap.GetOrRemove.cpp"
#undef DENSE_FUNCTION
#undef DENSE_TEMPLATE
#undef DENSE_REMOVE

template class MZ::DenseHashTable<uint64_t>;
template class MZ::DenseHashTable<uint32_t>;