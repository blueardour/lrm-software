//  ******************************************************************//
//  author: chenp
//  description: print fingerprint
//  version: 1.0
//	date: 2014-03-21
//  ******************************************************************//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "fingerprint.h"
#include "types.h"
#include "utils.h"


int main(int argc, char ** argv)
{
	FILE * fp;
	char * buffer, * ptr;
	char c;
	int i;
	u32 size, len;
	FType print[FPSize*2], * ptptr;

	len = 0;
	size = 1024;
	buffer = (char *)malloc(size);

	while((c=fgetc(stdin)) != EOF)
	{
		if(isgraph(c) == 0) continue;
	
		if(len == size)
		{
			size += 1024;
			buffer = (char *) realloc(buffer, size);
		}
		buffer[len++] = c;
	}

	fprintf(stdout, "Input Length: %d\r\n", len);
	ptr = buffer;
	ptptr = print;
	stampFinger(ptptr, ptr, len);
	for(i=0; i<FPSize; i++)	fprintf(stdout, "%d ", ptptr[i]);
	fprintf(stdout, "\r\n");

	if(argc == 2)
	{
		fp = fopen(argv[1], "r");
		if(fp == NULL)
		{
			printf("Can open file: %s\r\n", argv[1]);
			return -1;
		}

		c = fgetc(fp);
		if(c == '>') if(read2f_util(fp, '\n', 0, NULL, 0) != 0)
		{
			printf("Seq read error\r\n");
			return -2;
		}

		if(read2b_util(fp, EOF, 1, buffer, len) != 0)
		{
			printf("Seq read error\r\n");
			return -2;
		}

		ptptr = print + FPSize;
		stampFinger(ptptr, ptr, len);
		fprintf(stdout, " vs \r\n");
		for(i=0; i<FPSize; i++)	fprintf(stdout, "%d ", ptptr[i]);
		fprintf(stdout, "\r\n");

		fprintf(stdout, "Esitmate: %d \r\n", estimate(ptptr - FPSize, ptptr, FPSize));
	}

	free(buffer);
	return 0;
}



