
file(GLOB SOURCES "./*.cpp")
file(GLOB HEADERS CONFIGURE_DEPENDS "../include/*.h")

# Create library to enable separate test and run executables
add_library(chess_engine_lib ${SOURCES} ${HEADERS})

target_include_directories(chess_engine_lib PUBLIC ../include ../senjo)

target_compile_features(chess_engine_lib PRIVATE cxx_std_23)

target_precompile_headers(chess_engine_lib
  PRIVATE
    <algorithm>
    <atomic>
    <array>
    <compare>
    <cstdint>
    <filesystem>
    <fstream>
    <iostream>
    <iomanip>
    <iterator>
    <locale>
    <limits>
    <map>
    <numeric>
    <optional>
    <random>
    <ranges>
    <set>
    <span>
    <tl/expected.hpp>
    <csignal>
    <string>
    <sstream>
    <unordered_map>
    <unordered_set>
    <vector>
)

if(MSVC)
  target_compile_options(chess_engine_lib PRIVATE /W4 /WX)
else()
  target_compile_options(chess_engine_lib PRIVATE -Wall -Wextra -pedantic -Wno-missing-braces)
endif()

target_link_libraries(chess_engine_lib PRIVATE senjo)

# IDEs should put the headers in a nice place
source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header Files"
  FILES ${HEADERS})
