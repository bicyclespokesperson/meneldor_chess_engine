#!/usr/bin/env bash 

set -e

CLANG_FORMAT_EXE="/opt/homebrew/Cellar/llvm/13.0.1_1/bin/clang-format"

find ./src \( -name '*.cpp' -o -name '*.h' \) -exec "$CLANG_FORMAT_EXE" -i {} \;
find ./app \( -name '*.cpp' -o -name '*.h' \) -exec "$CLANG_FORMAT_EXE" -i {} \;
find ./include  \( -name '*.cpp' -o -name '*.h' \) -exec "$CLANG_FORMAT_EXE" -i {} \;
find ./tests \( -name '*.cpp' -o -name '*.h' \) -exec "$CLANG_FORMAT_EXE" -i {} \;


