bin/cgol: cgol.c gifenc.c gifenc.h
	clang-16 -Wall -Wextra -pedantic -o bin/cgol cgol.c gifenc.c

format: *.c *.h
	clang-format-16 -style='{BasedOnStyle: LLVM, IndentWidth: 4}' -i *.c *.h