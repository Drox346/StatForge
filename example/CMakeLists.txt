cmake_minimum_required(VERSION 4.0.0)

add_executable(
    example
    cpp/example.cpp
)

target_link_libraries(example PRIVATE StatForge)

add_executable(
    example_c
    c/example.c
)

target_link_libraries(example_c PRIVATE StatForge)

install(TARGETS example DESTINATION bin)
install(TARGETS example_c DESTINATION bin)
