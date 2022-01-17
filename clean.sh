#!/usr/bin/env bash

rm -rf ./CMakeScripts
rm -rf ./CMakeFiles 
rm -rf ./CMakeCache.txt 
rm -rf ./CTestTestfile.cmake 
rm -rf ./DartConfiguration.tcl 
rm -rf ./Debug 
rm -rf ./Testing 
rm -rf ./_deps 
rm -rf ./out 
rm -rf ./bin/*
rm -rf ./build
rm -rf ./compile_commands.json
rm -rf ./Makefile
rm -rf ./chess_engine.sln
rm -rf ./x64
rm -rf ./.vs

# Pipe to xargs to avoid warnings
# -delete can only delete empty directories
# -exec rm prints spurious errors when it tries to look inside deleted folders
find . -name "CMakeFiles" -type d -print0 | xargs -r0 -- rm -rf
find . -name "*.xcodeproj" -print0 | xargs -r0 -- rm -rf
find . -name "*.vcxproj*" -print0 | xargs -r0 -- rm -rf
find . -name "*.build" -print0 | xargs -r0 -- rm -rf
find . -name "cmake_install.cmake" -print0 | xargs -r0 -- rm -rf
find . -name "SharedPrecompiledHeaders" -print0 | xargs -r0 -- rm -rf
find . -name "Debug" -type d -print0 | xargs -r0 -- rm -rf

