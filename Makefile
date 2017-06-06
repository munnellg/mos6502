OBJS = $(wildcard src/*.c)

CC=gcc

LINKER_FLAGS=-lSDL2 -lncurses

COMPILER_FLAGS=-Wall -g

BINARY=mos

all : $(OBSJ)
	$(CC) $(COMPILER_FLAGS) $(OBJS) -o $(BINARY) $(LINKER_FLAGS)
