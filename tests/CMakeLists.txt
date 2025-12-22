cmake_minimum_required(VERSION 4.0.0)

add_executable(
    test_statforge
    
    main.cpp

    dsl/tokenizer.cpp
    dsl/parser.cpp
    dsl/evaluator.cpp

    spreadsheet/action_draft.cpp
    spreadsheet/cell_creation.cpp

    benchmarks/balanced_binary_tree.cpp
    benchmarks/chains.cpp
)

target_link_libraries(test_statforge PRIVATE StatForge doctest::doctest)

add_test(NAME test_statforge COMMAND test_statforge)

install(TARGETS test_statforge DESTINATION bin)