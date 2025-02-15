#include "DenseMap.hpp"

#define DENSE_TEMPLATE typename TKey, typename TValue
#define DENSE_FUNCTION bool __forceinline MZ::DenseMap::HashMap<TKey, TValue>::Add(TKey key, TValue value)
#include "DenseMap.Add.cpp"
#undef DENSE_FUNCTION
#undef DENSE_TEMPLATE

#define DENSE_UNIQUE
#define DENSE_TEMPLATE typename TKey, typename TValue
#define DENSE_FUNCTION void __forceinline MZ::DenseMap::HashMap<TKey, TValue>::AddUnique(TKey key, TValue value)
#include "DenseMap.Add.cpp"
#undef DENSE_FUNCTION
#undef DENSE_TEMPLATE
#undef DENSE_UNIQUE

#define DENSE_UPDATE
#define DENSE_TEMPLATE typename TKey, typename TValue
#define DENSE_FUNCTION bool __forceinline MZ::DenseMap::HashMap<TKey, TValue>::AddOrUpdate(TKey key, TValue value)
#include "DenseMap.Add.cpp"
#undef DENSE_FUNCTION
#undef DENSE_TEMPLATE
#undef DENSE_UPDATE

#define DENSE_TRY_GET
#define DENSE_TEMPLATE typename TKey, typename TValue
#define DENSE_FUNCTION bool __forceinline MZ::DenseMap::HashMap<TKey, TValue>::TryGetValue(TKey key, TValue& value)
#include "DenseMap.GetOrRemove.cpp"
#undef DENSE_FUNCTION
#undef DENSE_TEMPLATE
#undef DENSE_TRY_GET

#define DENSE_REMOVE
#define DENSE_TEMPLATE typename TKey, typename TValue
#define DENSE_FUNCTION bool __forceinline MZ::DenseMap::HashMap<TKey, TValue>::Remove(TKey key)
#include "DenseMap.GetOrRemove.cpp"
#undef DENSE_FUNCTION
#undef DENSE_TEMPLATE
#undef DENSE_REMOVE

template class MZ::DenseMap::HashMap<uint64_t, uint32_t>;
template class MZ::DenseMap::HashMap<uint32_t, uint32_t>;