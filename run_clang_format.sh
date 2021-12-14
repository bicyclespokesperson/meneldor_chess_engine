#!/usr/bin/env bash 

set -e

find ./src \( -name '*.cpp' -o -name '*.h' \) -exec clang-format -i {} \;
find ./app \( -name '*.cpp' -o -name '*.h' \) -exec clang-format -i {} \;
find ./include  \( -name '*.cpp' -o -name '*.h' \) -exec clang-format -i {} \;
find ./tests \( -name '*.cpp' -o -name '*.h' \) -exec clang-format -i {} \;


