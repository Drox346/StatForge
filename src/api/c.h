#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "../error/error.h"

typedef struct SF_Engine SF_Engine;

SF_Engine* sf_create_engine(void);
void sf_destroy_engine(SF_Engine*);

// SF_ErrorCode sf_create_value_cell(SF_Engine* engine, const char* name, double value);
// SF_ErrorCode sf_create_formula_cell(SF_Engine* engine, const char* name, const char* formula);

#ifdef __cplusplus
}
#endif