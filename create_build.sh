#!/usr/bin/env bash

create_build()
{

# Get platform
local readonly UNAME_OUT="$(uname -s)"
case "${UNAME_OUT}" in
  Linux*)     MACHINE=Linux;;
  Darwin*)    MACHINE=Mac;;
  CYGWIN*)    MACHINE=Windows;;
  MINGW*)     MACHINE=Windows;;
  *)          MACHINE="UNKNOWN:${UNAME_OUT}"
esac

CXX_COMPILER="clang++"

if [[ "$#" -ge 3 ]]; then
  >&2 echo "Usage: $(basename $0) [-d | -r] [-x for XCode build]"
  exit 1
fi

local readonly GIT_REPO_DIR=$(git rev-parse --show-toplevel)
local readonly BUILD_DIR="$GIT_REPO_DIR/_build"

rm -rf "$BUILD_DIR"
mkdir "$BUILD_DIR"
cd "$BUILD_DIR"

if [[ "$MACHINE" == "Windows" ]]; then
  # Need to use the CMake installed here, not the one installed via MinGW, to access the Visual Studio generator
  C:/Program\ Files/CMake/bin/cmake.exe -G "Visual Studio 17 2022" -A x64 ..
  exit
fi

BUILD_TYPE="DEBUG"
if [[ "$1" == "-r" ]]; then
  BUILD_TYPE="RELEASE"
fi
echo "Build type: ${BUILD_TYPE}"

CMAKE_ARGS="-D CMAKE_CXX_COMPILER=$CXX_COMPILER -D CMAKE_BUILD_TYPE=$BUILD_TYPE"

if [[ $(command -v "ccache") ]]; then
  CMAKE_ARGS="$CMAKE_ARGS -DCMAKE_CXX_COMPILER_LAUNCHER=ccache"
fi

if [[ "$1" == "-x" ]]; then
  CMAKE_ARGS="$CMAKE_ARGS -GXcode"
fi

cmake $CMAKE_ARGS ..
return $?
}

create_build "$@"
unset create_build
