cmake_minimum_required(VERSION 3.16)

add_executable(
    test_statforge
    
    main.cpp

    dsl/tokenizer.cpp
    dsl/parser.cpp
    dsl/evaluator.cpp

    rules/action_draft.cpp
    
    stat_kernel/node_creation.cpp
    stat_kernel/reset.cpp
)

target_link_libraries(test_statforge PRIVATE StatForge doctest::doctest)

add_test(NAME test_statforge COMMAND test_statforge)

add_executable(
    benchmark_statforge

    main.cpp
    benchmarks/balanced_binary_tree.cpp
    benchmarks/chains.cpp
)

target_link_libraries(benchmark_statforge PRIVATE StatForge doctest::doctest)

install(TARGETS test_statforge DESTINATION bin)
