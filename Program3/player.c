#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>


/*main function accepts args input filename, brightness value, output file name*/
int main(int argc, char *cdArgs[]){
	int readValue, i = 0, sum = 0;
	for(i = 0; i < 4; i++){
		read(0, &readValue, sizeof(int));
		sum = sum + readValue;
	}
	/*write the sum to a file descriptor*/
	write(1, &sum, sizeof(int));
	return 0;
}