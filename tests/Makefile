CC = gcc
SHELL := /usr/bin/env
.SHELLFLAGS := -S bash -c

CFLAGS = -std=c2x -Wall -Wextra -Wpedantic -pedantic-errors -fanalyzer -Wanalyzer-too-complex -ggdb -g3 -O0
LDLIBS +=
LDFLAGS +=

.PHONY: all c clean objs val
.DELETE_ON_ERROR:
.ONESHELL:

SRCS := $(wildcard *.c)
OBJS := $(SRCS:.c=.o)
TGTS := $(SRCS:.c=)

DEPDIR := .
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d

COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) -c

val:
	$(MAKE) all
	find . -iname "*.test" -exec valgrind --leak-check=yes ./{} \;


all: $(TGTS)
objs: $(OBJS)

task.test: task.test.o ../threadpool.o
tqn.test: tqn.test.o ../threadpool.o

../threadpool.o:
	$(MAKE) -C .. threadpool.o

%.o : %.c
%.o : %.c $(DEPDIR)/%.d | $(DEPDIR)
	$(COMPILE.c) $<

$(DEPDIR): ; @mkdir -p $@

DEPFILES := $(SRCS:%.c=$(DEPDIR)/%.d)
$(DEPFILES):

include $(wildcard $(DEPFILES))

c clean:
	rm -rvf $(TGTS) $(DEPFILES) $(wildcard *.o)


# Automatic dependency generation shamelessly borrowed and hamfistedly butchered from Paul D. Smith, via https://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
# Thanks Paul!