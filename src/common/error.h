#pragma once
#ifndef SF_ERROR_H
#define SF_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum SF_ErrorCode {
    SF_OK = 0,
    SF_UNKNOWN,
    SF_ERR_NOTHING_CHANGED,
    SF_ERR_INVALID_DSL,
    SF_ERR_CELL_ALREADY_EXISTS,
    SF_ERR_SELF_REFERENCE,
    SF_ERR_DEPENDENCY_LOOP,
    SF_ERR_DEPENDENCY_DOESNT_EXIST,
    SF_ERR_CELL_NOT_FOUND,
    SF_ERR_CELL_TYPE_MISMATCH,
} SF_ErrorCode;

typedef struct SF_Value {
    SF_ErrorCode error;
    double value;
} SF_Value;

/**
 * @brief sets an error message, overwriting the previous message
 * 
 * @return  
 */
void sf_set_error(const char* fmt, ...);

/**
 * @brief returns the message of the last error, consuming that message
 * 
 * @return const char* 
 */
const char* sf_last_error(void);

#ifdef __cplusplus
}
#endif

#endif // SF_ERROR_H
