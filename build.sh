#!/bin/sh

set -e

clang-format-16 -style='{BasedOnStyle: LLVM, IndentWidth: 4}' -i *.c *.h
clang-16 -Wall -Wextra -pedantic -o bin/cgol cgol.c gifenc.c
