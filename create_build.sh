#!/usr/bin/env bash

C_COMPILER="/usr/local/Cellar/llvm/13.0.0_1/bin/clang"
CXX_COMPILER="/usr/local/Cellar/llvm/13.0.0_1/bin/clang++"

if [[ "$#" -ge 3 ]]; then
  >&2 echo "Usage: $(basename $0) [-d | -r] [-x for XCode build]"
  exit 2
fi

if [[ "$1" == "-x" ]]; then
  ./clean.sh
  cmake -GXcode CMakeLists.txt
  exit
fi

BUILD_TYPE="DEBUG"
if [[ "$1" == "-r" ]]; then
  BUILD_TYPE="RELEASE"
fi
echo "Build type: ${BUILD_TYPE}"

cmake -D CMAKE_C_COMPILER="$C_COMPILER" -D CMAKE_CXX_COMPILER="$CXX_COMPILER" -D CMAKE_BUILD_TYPE="$BUILD_TYPE" CMakeLists.txt

