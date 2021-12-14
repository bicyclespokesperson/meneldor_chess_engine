add_executable(chess_game chess_game.cpp)
target_compile_features(chess_game PRIVATE cxx_std_20)
target_include_directories(chess_game PUBLIC ../include)
target_precompile_headers(chess_game
  PRIVATE
    <algorithm>
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
    <unordered_map>
    <vector>
)
target_link_libraries(perft PRIVATE chess_engine_lib )


add_executable(engine_main engine_main.cpp)
target_compile_features(engine_main PRIVATE cxx_std_20)
target_include_directories(engine_main PUBLIC ../include)
target_precompile_headers(engine_main
  PRIVATE
    <algorithm>
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
    <unordered_map>
    <vector>
)
target_link_libraries(engine_main PRIVATE chess_engine_lib senjo)
