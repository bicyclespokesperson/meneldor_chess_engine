#!/usr/bin/env bash 

set -e

if [[ $(command -v clang-format) ]]; then
  CLANG_FORMAT_EXE="clang-format"
else
  CLANG_FORMAT_EXE="$(find /opt/homebrew/Cellar/llvm -name clang-format)"
fi

find ./src \( -name '*.cpp' -o -name '*.h' \) -exec "$CLANG_FORMAT_EXE" -i {} \;
find ./app \( -name '*.cpp' -o -name '*.h' \) -exec "$CLANG_FORMAT_EXE" -i {} \;
find ./include  \( -name '*.cpp' -o -name '*.h' \) -exec "$CLANG_FORMAT_EXE" -i {} \;
find ./tests \( -name '*.cpp' -o -name '*.h' \) -exec "$CLANG_FORMAT_EXE" -i {} \;


