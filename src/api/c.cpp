#include "c.h"
#include "runtime/engine.hpp"

struct SF_Engine {
    statforge::runtime::EngineImpl engine;
};

namespace {

SF_ErrorCode validateEngine(SF_Engine* engine) {
    if (engine != nullptr) {
        return SF_OK;
    }
    sf_set_error("engine is null");
    return SF_ERR_INVALID_ENGINE_HANDLE;
}

SF_ErrorCode validateStringArg(const char* value, const char* name) {
    if (value != nullptr) {
        return SF_OK;
    }
    sf_set_error("%s is null", name);
    return SF_ERR_INTERNAL_INVALID_ENGINE_STATE;
}

} // namespace

SF_Engine* sf_create_engine() {
    return new SF_Engine{};
}

void sf_destroy_engine(SF_Engine* e) {
    delete e;
}

SF_ErrorCode sf_create_aggregator_cell(SF_Engine* engine, const char* name) {
    if (auto code = validateEngine(engine); code != SF_OK) {
        return code;
    }
    if (auto code = validateStringArg(name, "name"); code != SF_OK) {
        return code;
    }
    return engine->engine.createAggregatorCell(name);
}

SF_ErrorCode sf_create_formula_cell(SF_Engine* engine, const char* name, const char* formula) {
    if (auto code = validateEngine(engine); code != SF_OK) {
        return code;
    }
    if (auto code = validateStringArg(name, "name"); code != SF_OK) {
        return code;
    }
    if (auto code = validateStringArg(formula, "formula"); code != SF_OK) {
        return code;
    }
    return engine->engine.createFormulaCell(name, formula);
}

SF_ErrorCode sf_create_value_cell(SF_Engine* engine, const char* name, double value) {
    if (auto code = validateEngine(engine); code != SF_OK) {
        return code;
    }
    if (auto code = validateStringArg(name, "name"); code != SF_OK) {
        return code;
    }
    return engine->engine.createValueCell(name, value);
}

SF_ErrorCode sf_remove_cell(SF_Engine* engine, const char* name) {
    if (auto code = validateEngine(engine); code != SF_OK) {
        return code;
    }
    if (auto code = validateStringArg(name, "name"); code != SF_OK) {
        return code;
    }
    return engine->engine.removeCell(name);
}

SF_ErrorCode sf_set_cell_value(SF_Engine* engine, const char* name, double value) {
    if (auto code = validateEngine(engine); code != SF_OK) {
        return code;
    }
    if (auto code = validateStringArg(name, "name"); code != SF_OK) {
        return code;
    }
    return engine->engine.setCellValue(name, value);
}

SF_ErrorCode sf_set_cell_formula(SF_Engine* engine, const char* name, const char* formula) {
    if (auto code = validateEngine(engine); code != SF_OK) {
        return code;
    }
    if (auto code = validateStringArg(name, "name"); code != SF_OK) {
        return code;
    }
    if (auto code = validateStringArg(formula, "formula"); code != SF_OK) {
        return code;
    }
    return engine->engine.setCellFormula(name, formula);
}

SF_ErrorCode sf_set_cell_dependency(SF_Engine* engine, const char* name, const char* dependencies) {
    if (auto code = validateEngine(engine); code != SF_OK) {
        return code;
    }
    if (auto code = validateStringArg(name, "name"); code != SF_OK) {
        return code;
    }
    if (auto code = validateStringArg(dependencies, "dependencies"); code != SF_OK) {
        return code;
    }
    return engine->engine.setCellDependency(name, dependencies);
}

SF_ErrorCode sf_get_cell_value(SF_Engine* engine, const char* name, double* out_value) {
    if (auto code = validateEngine(engine); code != SF_OK) {
        return code;
    }
    if (auto code = validateStringArg(name, "name"); code != SF_OK) {
        return code;
    }
    if (out_value == nullptr) {
        sf_set_error("out_value is null");
        return SF_ERR_INTERNAL_INVALID_ENGINE_STATE;
    }
    return engine->engine.getCellValue(name, *out_value);
}

SF_Value sf_get_cell_value2(SF_Engine* engine, const char* name) {
    SF_Value result = {
        .error = SF_ERR_INTERNAL_INVALID_ENGINE_STATE,
        .value = 0.0,
    };

    SF_ErrorCode code = sf_get_cell_value(engine, name, &result.value);
    result.error = code;
    if (code != SF_OK) {
        result.value = 0.0;
    }

    return result;
}

void sf_reset_engine(SF_Engine* engine) {
    if (validateEngine(engine) != SF_OK) {
        return;
    }
    engine->engine.reset();
}

void sf_evaluate_engine(SF_Engine* engine) {
    if (validateEngine(engine) != SF_OK) {
        return;
    }
    engine->engine.evaluate();
}
