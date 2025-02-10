#include "DenseMap.hpp"

#define DENSE_HASHTABLE
#define DENSE_TEMPLATE typename TKey
#define DENSE_FUNCTION bool __forceinline MZ::DenseMap::HashTable<TKey>::Add(TKey key)
#include "DenseMap.Add.cpp"
#undef DENSE_FUNCTION
#undef DENSE_TEMPLATE
#undef DENSE_HASHTABLE

#define DENSE_UNIQUE
#define DENSE_HASHTABLE
#define DENSE_TEMPLATE typename TKey
#define DENSE_FUNCTION void __forceinline MZ::DenseMap::HashTable<TKey>::AddUnique(TKey key)
#include "DenseMap.Add.cpp"
#undef DENSE_FUNCTION
#undef DENSE_TEMPLATE
#undef DENSE_HASHTABLE
#undef DENSE_UNIQUE

#define DENSE_HASHTABLE
#define DENSE_TEMPLATE typename TKey
#define DENSE_FUNCTION bool __forceinline MZ::DenseMap::HashTable<TKey>::Contains(TKey key)
#include "DenseMap.GetOrRemove.cpp"
#undef DENSE_FUNCTION
#undef DENSE_TEMPLATE
#undef DENSE_HASHTABLE

#define DENSE_REMOVE
#define DENSE_HASHTABLE
#define DENSE_TEMPLATE typename TKey
#define DENSE_FUNCTION bool __forceinline MZ::DenseMap::HashTable<TKey>::Remove(TKey key)
#include "DenseMap.GetOrRemove.cpp"
#undef DENSE_FUNCTION
#undef DENSE_TEMPLATE
#undef DENSE_HASHTABLE
#undef DENSE_REMOVE

template class MZ::DenseMap::HashTable<uint64_t>;
template class MZ::DenseMap::HashTable<uint32_t>;