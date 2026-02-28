#pragma once
#ifndef SF_ERROR_H
#define SF_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum SF_ErrorCode {
    // No error
    SF_OK = 0,


    /*** Cell manipulation ***/
    /*
        Errors in this category indicate that an operation was attempted
        but rejected. No changes were applied to the spreadsheet state.
    */

    // DSL input was ill-formed.
    SF_ERR_INVALID_DSL = 100,

    // Attempted to create a cell with an ID that already exists.
    SF_ERR_CELL_ALREADY_EXISTS,

    // Attempted to set a formula that references itself.
    SF_ERR_SELF_REFERENCE,

    // Requested dependency would introduce a direct or indirect cycle.
    SF_ERR_DEPENDENCY_LOOP,

    // Attempted to reference a cell that does not exist.
    SF_ERR_DEPENDENCY_DOESNT_EXIST,

    // Attempted to remove a cell required by a formula cell.
    SF_ERR_DEPENDENT_FORMULA_CELL,

    // Attempted to access a cell that does not exist.
    SF_ERR_CELL_NOT_FOUND,

    // Attempted to perform an action on a cell with a cell type that doesn't support that action.
    SF_ERR_CELL_TYPE_MISMATCH,


    /*** Evaluation ***/
    /*
        Errors produced during evaluation. These do not modify the spreadsheet
        but reflect failures during expression execution.
    */

    // Divide by 0 error during evaluation.
    SF_ERR_EVAL_DIV_BY_ZERO = 200,

    // Overflow error during evaluation.
    SF_ERR_EVAL_OVERFLOW,

    // Evaluation resulted in NaN (not a number)
    SF_ERR_EVAL_NAN,


    /*** Other ***/

    // Internal error. Causes the engine to enter an invalid state.
    // THIS CANNOT BE HEALED
    // Please report it to the dev if you encounter this error. Provide
    // error details from "sf_last_error()" and a full .dot graph export.
    SF_ERR_INTERNAL_INVALID_ENGINE_STATE = 1000,

    // API call received a null or invalid engine handle.
    SF_ERR_INVALID_ENGINE_HANDLE,
} SF_ErrorCode;

typedef struct SF_Value {
    SF_ErrorCode error;
    double value;
} SF_Value;

/**
 * @brief sets an error message. overwrites the previous message.
 * 
 * @return  
 */
void sf_set_error(const char* fmt, ...);

/**
 * @brief returns the message of the last error and consumes that message.
 *        pointer is valid until a new error is written.
 * 
 * @return const char* 
 */
const char* sf_last_error(void);

#ifdef __cplusplus
}
#endif

#endif // SF_ERROR_H
