#!/usr/bin/env bash

build_project()
{

# Get platform
local readonly UNAME_OUT="$(uname -s)"
case "${UNAME_OUT}" in
  Linux*)     MACHINE=Linux;;
  Darwin*)    MACHINE=Mac;;
  CYGWIN*)    MACHINE=Cygwin;;
  MINGW*)     MACHINE=MinGw;;
  *)          MACHINE="UNKNOWN:${UNAME_OUT}"
esac

local readonly GIT_REPO_DIR=$(git rev-parse --show-toplevel)
local readonly BUILD_DIR="$GIT_REPO_DIR/_build"
if [ ! -d "$BUILD_DIR" ]; then
  >&2 echo "Build directory does not exist. Run create_build.sh first to generate build files."
  exit 1
fi
cd "$BUILD_DIR"

if [ "$MACHINE" == "Mac" ]; then 
  PROCESSOR_COUNT=$(sysctl -n hw.ncpu)
elif [ "$MACHINE" == "Linux" ]; then 
  PROCESSOR_COUNT=$(nproc --all)
else
  # Could add windows support eventually, though the project will usually be built via visual studio
  PROCESSOR_COUNT=1
fi

make -j "$PROCESSOR_COUNT"
return $?
}

time build_project
