cmake_minimum_required(VERSION 4.0.0)

add_executable(
    tests
    main.cpp
)

target_link_libraries(tests PRIVATE StatForge doctest::doctest)

add_test(NAME statforge_tests COMMAND tests)

install(TARGETS tests DESTINATION bin)