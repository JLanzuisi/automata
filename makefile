CC=gcc
PNAME=automata

bin/$(PNAME): $(PNAME).c gifenc.c gifenc.h
	$(CC) -Wall -Wextra -pedantic -ggdb -L. -o bin/$(PNAME) $(PNAME).c gifenc.c
