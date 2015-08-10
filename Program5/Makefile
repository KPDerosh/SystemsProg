all: cclient cserver
	
cclient:
	gcc -ansi -pedantic cclient.c -o cclient
cserver:
	gcc -ansi -pedantic -pthread cserver.c -o cserver

clean:
	rm -f cclient cserver
