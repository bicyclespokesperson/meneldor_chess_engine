#!/usr/bin/env bash

create_build()
{

# Get platform
UNAME_OUT="$(uname -s)"
case "${UNAME_OUT}" in
  Linux*)     MACHINE=Linux;;
  Darwin*)    MACHINE=Mac;;
  CYGWIN*)    MACHINE=Windows;;
  MINGW*)     MACHINE=Windows;;
  *)          MACHINE="UNKNOWN:${UNAME_OUT}"
esac

CXX_COMPILER="/usr/local/Cellar/llvm/13.0.0_1/bin/clang++"

if [[ "$MACHINE" == "Windows" ]]; then
  # Need to use the CMake installed here, not the one installed via MinGW, to access the Visual Studio generator
  C:/Program\ Files/CMake/bin/cmake.exe -G "Visual Studio 17 2022" -A x64 CMakeLists.txt
  exit
fi

if [[ "$#" -ge 3 ]]; then
  >&2 echo "Usage: $(basename $0) [-d | -r] [-x for XCode build] [-v for Visual Studio build]"
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

cmake -D CMAKE_CXX_COMPILER="$CXX_COMPILER" -D CMAKE_BUILD_TYPE="$BUILD_TYPE" CMakeLists.txt

}

create_build "$@"
unset create_build
