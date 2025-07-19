#pragma once
#ifndef SF_ERROR_H
#define SF_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum SF_Error {
    SF_OK = 0,
    SF_ERR_NOTHING_CHANGED,
    SF_ERR_INVALID_DSL,
    SF_ERR_CELL_ALREADY_EXISTS,
    SF_ERR_SELF_REFERENCE,
    SF_ERR_DEPENDENCY_LOOP,
    SF_ERR_DEPENDENCY_DOESNT_EXIST,
    SF_ERR_CELL_NOT_FOUND,
    SF_ERR_CELL_TYPE_MISMATCH,
} SF_Error;

const char* sf_strerror(SF_Error err);

#ifdef __cplusplus
}
#endif

#endif // SF_ERROR_H