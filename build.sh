#!/bin/sh

#goimports -w . && staticcheck && go build -o bin/conway

cc -Wall -Wextra -pedantic -o bin/cgol cgol.c gifenc.c
