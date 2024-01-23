#!/bin/sh

goimports -w . && staticcheck && go build -o bin/conway
