#!/bin/bash
set -e

cd "$(dirname "$0")/../main"
pwd
find -iname *.h -o -iname *.cpp -o -iname *.c | xargs clang-format -i

cd "../pctool"
pwd
clang-format -i *.cpp

cd "../common"
pwd
clang-format -i *.h

echo "Finished"
