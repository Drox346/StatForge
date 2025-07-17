cmake_minimum_required(VERSION 4.0.0)

file(GLOB_RECURSE SOURCES "*.cpp")
add_library(StatForge STATIC ${SOURCES})

target_include_directories(StatForge PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_property(TARGET StatForge PROPERTY COMPILE_WARNING_AS_ERROR ON)
target_compile_options(StatForge PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
  $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wall -Wextra -pedantic -pedantic-errors>
)