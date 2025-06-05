CC = gcc
SHELL := /usr/bin/env
.SHELLFLAGS := -S bash -c

CFLAGS = -std=c2x -Wall -Wextra -Wpedantic -pedantic-errors -fanalyzer -Wanalyzer-too-complex -ggdb -g3 -O0

CFLAGS +=
LDLIBS +=
LDFLAGS +=

.PHONY: all c clean
.DELETE_ON_ERROR:
.ONESHELL:

threadpool.o: threadpool.c threadpool.h

c clean:
	rm -rvf $(wildcard *.o)