add_executable(chess_game chess_game.cpp)
target_compile_features(chess_game PRIVATE cxx_std_20)
target_include_directories(chess_game PUBLIC ../include)
target_precompile_headers(chess_game
  PRIVATE
    <algorithm>
    <array>
    <compare>
    <cstdint>
    <filesystem>
    <fstream>
    <iostream>
    <map>
    <numeric>
    <random>
    <set>
    <span>
    <string>
    <sstream>
    <tl/expected.hpp>
    <unordered_map>
    <vector>
)
target_link_libraries(chess_game PRIVATE senjo chess_engine_lib )


add_executable(perft perft.cpp)
target_compile_features(perft PRIVATE cxx_std_20)
target_include_directories(perft PUBLIC ../include)
target_precompile_headers(perft
  PRIVATE
    <algorithm>
    <array>
    <compare>
    <cstdint>
    <filesystem>
    <fstream>
    <iostream>
    <map>
    <numeric>
    <random>
    <set>
    <span>
    <string>
    <sstream>
    <tl/expected.hpp>
    <unordered_map>
    <vector>
)
target_link_libraries(perft PRIVATE chess_engine_lib )


add_executable(meneldor engine_main.cpp)
target_compile_features(meneldor PRIVATE cxx_std_20)
target_include_directories(meneldor PUBLIC ../include)
target_precompile_headers(meneldor
  PRIVATE
    <algorithm>
    <array>
    <compare>
    <cstdint>
    <ctime>
    <filesystem>
    <fstream>
    <iomanip>
    <iostream>
    <map>
    <numeric>
    <random>
    <set>
    <span>
    <sstream>
    <string>
    <tl/expected.hpp>
    <unordered_map>
    <vector>
)
target_link_libraries(meneldor PRIVATE chess_engine_lib senjo)
