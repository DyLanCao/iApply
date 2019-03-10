#include <stdio.h>  

void main() {
	FILE *fp;
	int length;

	fp = fopen("file.txt", "r");
	fseek(fp, 0, SEEK_END);

	length = ftell(fp);

	fclose(fp);
	printf("Size of file: %d bytes", length);

}
