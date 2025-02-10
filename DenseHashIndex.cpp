#include "DenseMap.hpp"

#define DENSE_HASHINDEX
#define DENSE_TEMPLATE typename TKey
#define DENSE_FUNCTION uint32_t __forceinline MZ::DenseMap::HashIndex<TKey>::AddOrGet(TKey key)
#include "DenseMap.Add.cpp"
#undef DENSE_FUNCTION
#undef DENSE_TEMPLATE
#undef DENSE_HASHINDEX

#define DENSE_UNIQUE
#define DENSE_HASHINDEX
#define DENSE_TEMPLATE typename TKey
#define DENSE_FUNCTION void __forceinline MZ::DenseMap::HashIndex<TKey>::AddUnique(TKey key)
#include "DenseMap.Add.cpp"
#undef DENSE_FUNCTION
#undef DENSE_TEMPLATE
#undef DENSE_HASHINDEX
#undef DENSE_UNIQUE

#define DENSE_HASHINDEX
#define DENSE_TEMPLATE typename TKey
#define DENSE_FUNCTION uint32_t __forceinline MZ::DenseMap::HashIndex<TKey>::Get(TKey key)
#include "DenseMap.GetOrRemove.cpp"
#undef DENSE_FUNCTION
#undef DENSE_TEMPLATE
#undef DENSE_HASHINDEX

template class MZ::DenseMap::HashIndex<uint64_t>;
template class MZ::DenseMap::HashIndex<uint32_t>;