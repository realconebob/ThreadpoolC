CC = gcc
SHELL := /usr/bin/env
.SHELLFLAGS := -S bash -c

CFLAGS = -std=c2x -Wall -Wextra -Wpedantic -pedantic-errors -fanalyzer -Wanalyzer-too-complex -ggdb -g3 -O0
LDLIBS +=
LDFLAGS +=

.PHONY: all c clean val
.DELETE_ON_ERROR:
.ONESHELL:

all: main
main: main.o threadpool.o threadpool.h

main.o: main.c threadpool.h
threadpool.o: threadpool.c threadpool.h


c clean:
	rm -rvf main $(wildcard *.o) 

val:
	$(MAKE) all
	valgrind --leak-check=yes ./main