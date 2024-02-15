CC=clang-16

.PHONY=format

bin/cgol: cgol.c gifenc.c gifenc.h
	$(CC) -Wall -Wextra -pedantic -o bin/cgol cgol.c gifenc.c

hellolua.so: lua_hello.h
	$(CC) -c -Wall -Werror -fpic lua_hello.h
	$(CC) -shared -o hellolua.so lua_hello.h.gch
	rm lua_hello.h.gch

format: *.c *.h
	clang-format-16 -style='{BasedOnStyle: LLVM, IndentWidth: 4}' -i *.c *.h