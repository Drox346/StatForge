cmake_minimum_required(VERSION 4.0.0)
project(StatForge)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(src)

option(TESTING "Enable or disable testing" OFF)
option(EXAMPLES "Enable or disable building examples" ON)

if(EXAMPLES)
    add_subdirectory(example)
endif()

if(TESTING)
    include(CTest)
    enable_testing()

    add_subdirectory(extern)
    add_subdirectory(tests)
endif()