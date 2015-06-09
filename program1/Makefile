CC=gcc
CFLAGS=-ansi -pedantic-errors -Wall

default: all	
	$(CC) -o cppmbright cppmbright.o

all:		cppmbright.o

cppmbright.o:	cppmbright.c
	$(CC) $(CFLAGS) -c cppmbright.c

.PHONY: clean

clean:
	rm -rf cppmbright cppmbright.o
