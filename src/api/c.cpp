#include "c.h"
#include "dsl/error.hpp"
#include "error/error.h"
#include "runtime/engine.hpp"

#include <exception>

struct SF_Engine {
    statforge::runtime::EngineImpl engine;
};

SF_Engine* sf_create_engine() {
    return new SF_Engine{};
}

void sf_destroy_engine(SF_Engine* e) {
    delete e;
}

// SF_ErrorCode sf_create_value_cell(SF_Engine* engine, const char* name, double value) {
//     try {
//         engine->engine.kernel().createValueCell(name, value);
//         return SF_OK;
//     } catch (const std::exception& ex) {
//         return SF_ERR_CELL_ALREADY_EXISTS;
//     }
// }

// SF_ErrorCode sf_create_formula_cell(SF_Engine* engine, const char* name, const char* formula) {
//     try {
//         engine->engine.kernel().createFormulaCell(name, formula);
//         return SF_OK;
//     } catch (const statforge::dsl::DslError& ex) {
//         return SF_ERR_INVALID_DSL;
//     }
// }