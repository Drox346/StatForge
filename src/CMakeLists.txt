cmake_minimum_required(VERSION 4.0.0)

file(GLOB_RECURSE SOURCES
  CONFIGURE_DEPENDS
  "*.cpp"
  "*.c"
)
add_library(StatForge SHARED ${SOURCES})
add_library(StatForge_static STATIC ${SOURCES})

target_include_directories(StatForge PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(StatForge_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set_property(TARGET StatForge PROPERTY COMPILE_WARNING_AS_ERROR ON)
set_property(TARGET StatForge_static PROPERTY COMPILE_WARNING_AS_ERROR ON)

target_compile_options(StatForge PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
  $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wall -Wextra -pedantic -pedantic-errors>
)
target_compile_options(StatForge_static PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
  $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wall -Wextra -pedantic -pedantic-errors>
)

set_target_properties(StatForge PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    OUTPUT_NAME "StatForge"
)
set_target_properties(StatForge_static PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    OUTPUT_NAME "StatForge"
)