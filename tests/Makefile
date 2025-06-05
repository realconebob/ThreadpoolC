CC = gcc
SHELL := /usr/bin/env
.SHELLFLAGS := -S bash -c

CFLAGS = -std=c2x -Wall -Wextra -Wpedantic -pedantic-errors -fanalyzer -Wanalyzer-too-complex -ggdb -g3 -O0

CFLAGS +=
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
RM.shell = rm -v

val:
	$(MAKE) all
	find . -iname "*.test" -exec valgrind --leak-check=yes {} \;


all: $(TGTS)
objs: $(OBJS)

# Tried making a custom rule for these but it kept breaking. I'll get to it eventually
task.test: task.test.o ../threadpool.o
tqn.test: tqn.test.o ../threadpool.o
taskqueue.test: taskqueue.test.o ../threadpool.o
cleanup.test: cleanup.test.o ../threadpool.o
ctqueue.test: ctqueue.test.o ../threadpool.o

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
	-if [ ! -z "$(DEPDIR)" ] && [ "$(DEPDIR)" != "." ]; then
		$(RM.shell) $(DEPDIR)
	fi
	$(RM.shell) -f $(TGTS) $(DEPFILES) $(wildcard *.o)
	$(MAKE) -C .. clean

# Automatic dependency generation shamelessly borrowed and hamfistedly butchered from Paul D. Smith, via https://make.mad-scientist.net/papers/advanced-auto-dependency-generation/
# Thanks Paul!