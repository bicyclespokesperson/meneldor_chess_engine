# $ cmake CMakeLists.txt
# $ make
#
# Much of this file comes from: https://cliutils.gitlab.io/modern-cmake/

cmake_minimum_required(VERSION 3.16)

project (chess_engine VERSION 0.1 LANGUAGES CXX)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

  # Create a compile_commands.json file
  set(CMAKE_EXPORT_COMPILE_COMMANDS True)

  set(CMAKE_CXX_STANDARD 20)

  # Let's ensure -std=c++xx instead of -std=g++xx
  set(CMAKE_CXX_EXTENSIONS OFF)

  # Place the complied executable in the bin directory
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

  #set(CMAKE_CXX_CLANG_TIDY 
  #clang-tidy; 
  #-header-filter=./include)

  # Let's nicely support folders in IDEs
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  # Testing only available if this is the main app
  # Note this needs to be done in the main CMakeLists
  # since it calls enable_testing, which must be in the
  # main CMakeLists.
  # This sets BUILD_TESTING=ON
  include(CTest)

endif()

include(FetchContent)

# Add the CMakeLists.txt files in the following directories
add_subdirectory(src)
add_subdirectory(app)
add_subdirectory(senjo/senjo)
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(tests)
endif()



