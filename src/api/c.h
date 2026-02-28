#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "../error/error.h"

typedef struct SF_Engine SF_Engine;

SF_Engine* sf_create_engine(void);
void sf_destroy_engine(SF_Engine*);

SF_ErrorCode sf_create_aggregator_cell(SF_Engine* engine, const char* name);
SF_ErrorCode sf_create_formula_cell(SF_Engine* engine, const char* name, const char* formula);
SF_ErrorCode sf_create_value_cell(SF_Engine* engine, const char* name, double value);
SF_ErrorCode sf_remove_cell(SF_Engine* engine, const char* name);
SF_ErrorCode sf_set_cell_value(SF_Engine* engine, const char* name, double value);
SF_ErrorCode sf_set_cell_formula(SF_Engine* engine, const char* name, const char* formula);
SF_ErrorCode sf_set_cell_dependency(SF_Engine* engine, const char* name, const char* dependencies);
SF_ErrorCode sf_get_cell_value(SF_Engine* engine, const char* name, double* out_value);
SF_Value sf_get_cell_value2(SF_Engine* engine, const char* name);

void sf_reset_engine(SF_Engine* engine);
void sf_evaluate_engine(SF_Engine* engine);

#ifdef __cplusplus
}
#endif
