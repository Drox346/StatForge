cmake_minimum_required(VERSION 4.0.0)

file(GLOB_RECURSE SOURCES "*.cpp")
add_library(StatForge STATIC ${SOURCES})

target_include_directories(StatForge PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})