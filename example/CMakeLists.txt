cmake_minimum_required(VERSION 4.0.0)

add_executable(
    example
    main.cpp
)

target_link_libraries(example PRIVATE StatForge)

install(TARGETS example DESTINATION bin)