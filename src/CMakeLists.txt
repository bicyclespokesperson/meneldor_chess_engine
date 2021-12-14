
file(GLOB SOURCES "./*.cpp")

# Create library to enable separate test and run executables
add_library(chess_engine_lib ${SOURCES})

target_include_directories(chess_engine_lib PUBLIC ../include ../senjo)

target_compile_features(chess_engine_lib PRIVATE cxx_std_20)

target_precompile_headers(chess_engine_lib
  PRIVATE
    <algorithm>
    <atomic>
    <compare>
    <cstdint>
    <filesystem>
    <fstream>
    <iostream>
    <iomanip>
    <locale>
    <limits>
    <map>
    <numeric>
    <numeric>
    <random>
    <set>
    <span>
    <signal.h>
    <string>
    <sstream>
    <unistd.h>    
    <unordered_map>
    <unordered_set>
    <vector>
)

if(MSVC)
  target_compile_options(chess_engine_lib PRIVATE /W4 /WX)
else()
  target_compile_options(chess_engine_lib PRIVATE -Wall -Wextra -pedantic -Wno-missing-braces)
endif()

file(GLOB HEADER_LIST CONFIGURE_DEPENDS "./include/modern/*.h")
#file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${ModernCMakeExample_SOURCE_DIR}/include/modern/*.hpp")

# IDEs should put the headers in a nice place
source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header Files"
  FILES ${HEADER_LIST})
