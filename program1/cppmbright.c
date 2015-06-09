#include <stdio.h>

int main(int argc, char *argvp[]){
	
	if(argc != 3){
		return -1;
	}
	char buff[5000];
	/*open the file of the image.*/
	int inputImage = open("lena.ppm", O_RDONLY, 0644);
	int finalImage = open(argvp[2], O_RDWR|O_TRUNC|O_CREAT, 0644);
	if(image == -1){
		return -1;
	}

	int sizeOfImage = read(inputImage, void *buf, 1);

	char* image1DArray ;
	int brightnessValue = argvp[1];

	return 0;
}
