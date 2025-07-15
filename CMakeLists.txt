cmake_minimum_required(VERSION 4.0.0)
project(StatForge)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_BUILD_TYPE Release)
# set(CMAKE_CXX_FLAGS_RELEASE "-O2")
# set(CMAKE_CXX_FLAGS_RELEASE "-fno-omit-frame-pointer")

# CPP23 MODULES?
# set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD ON)

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