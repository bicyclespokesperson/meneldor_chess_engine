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


if [ "$MACHINE" == "Mac" ]; then 
  PROCESSOR_COUNT=$(sysctl -n hw.ncpu)
elif [ "$MACHINE" == "Linux" ]; then 
  PROCESSOR_COUNT=$(nproc --all)
else
  #TODO: Windows support (though on windows this will usually be built through Visual Studio)
  PROCESSOR_COUNT=1
fi

make -j "$PROCESSOR_COUNT"
}

time build_project
unset build_project
