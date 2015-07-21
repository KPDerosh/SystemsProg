default: all	
	
all: dealer.o player.o

dealer.o:	dealer.c
	gcc -ansi -pedantic -o dealer.out dealer.c

program.o: player.c
	gcc -ansi -pedantic -o player player.c

.PHONY: clean

clean:
	rm -rf dealer.out
	rm -rf player
