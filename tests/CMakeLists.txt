
# Tests need to be added as executables first
add_executable(tests general_tests.cpp engine_tests.cpp perft_tests.cpp)
set_property(TARGET tests PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
set_property(TARGET tests PROPERTY XCODE_GENERATE_SCHEME TRUE)
set_property(TARGET tests PROPERTY XCODE_SCHEME_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# Compiler settings for the test code

include_directories(../include)

target_compile_features(tests PRIVATE cxx_std_23)

target_precompile_headers(tests
  PRIVATE
    <algorithm>
    <array>
    <atomic>
    <compare>
    <cstdint>
    <ctime>
    <filesystem>
    <fstream>
    <iostream>
    <iterator>
    <map>
    <numeric>
    <optional>
    <random>
    <ranges>
    <set>
    <span>
    <string>
    <sstream>
    <tl/expected.hpp>
    <unordered_map>
    <vector>
)

# Need to link with the main library to build/test it, and also link with Catch2
target_link_libraries(tests PRIVATE chess_engine_lib senjo Catch2::Catch2)

# Add this so we can use "make test"
# CMAKE_SOURCE_DIR == the top level of the tree
# Name = arg to make test
# Command = executable make test will run
add_test(NAME general_tests COMMAND tests  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} )

