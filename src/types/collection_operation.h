#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum SF_CollectionOperation {
    SF_COLLECTION_OP_SUM = 0,
    SF_COLLECTION_OP_PRODUCT,
    SF_COLLECTION_OP_MEDIAN,
    SF_COLLECTION_OP_AVERAGE,
    SF_COLLECTION_OP_MIN,
    SF_COLLECTION_OP_MAX,
    SF_COLLECTION_OP_COUNT,
} SF_CollectionOperation;

#ifdef __cplusplus
}
#endif
