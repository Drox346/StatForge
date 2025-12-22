Project: StatForge

1. Project Overview  
Goal:  
    - Spreadsheet-like computation engine as a C++ library for stat-heavy systems (e.g., build planners, ARPGs, simulations).  
    - Minimize complexity when defining and maintaining complex stat interactions.
    - High-performance core using value caching, dependency tracking, and selective reevaluation.  
    - Scales to large graphs with deep or wide dependency structures, validated with high-fanout stress tests.  

2. Core Design Principles  
    - Evaluation driven solely by external state.
    - Single-direction interaction: external conditions -> mutate cells; never the reverse.  
    - No runtime recursion or implicit reactivity: evaluation runs in fixed, explicit passes.
    - Deterministic spreadsheet logic isolated from all domain-specific control logic.  
    - Uniform numeric type (`double`) eliminates type-system overhead.  
    - No multithreading
    - Minimal but still expressive API. Avoiding usage errors and bad practices by making them impossible to design in the first place.
    - Strict cell-mutation-time checks to guarantee spreadsheet correctness. Allows skipping evaluation time checks for higher performance.

3. Architecture Overview
    - Cell-centric DAG with Value, Aggregator, and Formula cell types.  
    - Strict value caching; unchanged inputs skip computation.  
    - Exact dirty-flag propagation; only affected cells are reevaluated.
    - Evaluate when cell value is read and it's dirty. Never allow reading a dirty value.
    - Deterministic, topologically ordered evaluation phases.
    - Built-in debug export (e.g., `.exportGraph()` to .dot).
    - Exposing C-API for FFI. Allows other languages like Lua, Go or Python to use this engine.
    - undo/resetTo functionality to roll back on error or if user wants to. (e.g. checking what bonuses an item provides and rolling back to previous state)
    - std::expected error propagation within the engine. Each api call returns an error code and user can request the info message from the last error.
    - no exceptions

3. Condition -> Action System  
Purpose:  
    - Declaratively link external states to spreadsheet mutations (set value, replace formula, etc.).  

Architecture:  
    - Flat map of ConditionID + Value -> Action.  
    - Rules fire only on condition changes and run in batch; no recursive chains.  
    - Supports rule scopes for context switching (e.g., "MinionBuild").  

4. Best Practices and Avoided Pitfalls  
    - Don't feed spreadsheet results back into condition logic.
    - Only call evaluate() when doing big changes like a context switch or during startup. Prefer to let the engine decide what and when to update.
    - Avoid abstract `enabledIf`-style meta-logic unless proven necessary. Prefer manipulating cells to reflect that logic.
    - Aways check the API call's return code for errors. Failed calls are rolled back automatically so that the engine always stays in a valid state. Call "sf_last_error()" to get additional information on the last error.
    - Pre-declare all dependencies. Creation of a cell with non existing dependencies will be declined.
    - If you do bulk changes, then use the batch helpers.

5. Action Points  
Interface & Architecture  
    - Spreadsheet core implemented with dependency graph, caching, and precise reevaluation.  
    - Public API is minimal (`setValue`, `setFormula`, queries).  
    - Debug export functions are available for graph inspection.  

Condition–Action System  
    - ConditionRegistry and rule engine implemented.  
    - Rule scopes can be enabled, disabled, or removed at runtime.  
    - Rule evaluation is batch-triggered on condition updates.    

Planning & Future Proofing  
    - Maintain deterministic, side-effect-free internals.
    - Support for more functions like min, max, pow, clamp etc
    - Add reactive or introspective features only if real use cases demand them.  
    - Profile real workloads before expanding rule or condition complexity.  
    - Future extensions: rule builder APIs, DSL enhancements, optional metadata validation.
