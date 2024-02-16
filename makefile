CC=clang-16
PNAME=automata

.PHONY=format

bin/$(PNAME): $(PNAME).c gifenc.c gifenc.h
	$(CC) -Wall -Wextra -pedantic -L. -o bin/$(PNAME) $(PNAME).c gifenc.c

format: *.c *.h
	clang-format-16 -style='{BasedOnStyle: LLVM, IndentWidth: 4}' -i *.c *.h
