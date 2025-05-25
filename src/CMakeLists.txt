cmake_minimum_required(VERSION 4.0.0)

add_library(StatForge STATIC
    core.cpp
)

target_include_directories(StatForge PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})