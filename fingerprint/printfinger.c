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

int read_stream(FILE * fp, char * buffer, int * blen)
{
	char c;
	int slen;

	c = fgetc(fp);
	if(c == '>')
	{
		if(read2f_util(fp, '\n', 0, NULL, 0) != 0)
		{
			printf("Seq read error-1\r\n");
			return -2;
		}
		slen = 0;
	}
	else
	{
		buffer[0] = c;
		slen = 1;
	}

	while((c=fgetc(fp)) != EOF)
	{
		if(isgraph(c) == 0) continue;

		if(*blen == slen)
		{
			*blen += 1024;
			buffer = (char *) realloc(buffer, *blen);
		}
		buffer[slen++] = c;
	}

	return slen;
}

int main(int argc, char ** argv)
{
	FILE * fp;
	char * buffer, * ptr;
	int i, len1, len2, size;
	FType print[FPSize*2], * ptptr;

	size = 1024;
	buffer = (char *)malloc(size);
	len1 = read_stream(stdin, buffer, &size);
	fprintf(stdout, "Input Length: %d\r\n", len1);
	if(len1 < 0)
	{
		free(buffer);
		return len1;
	}

	ptr = buffer;
	ptptr = print;
	stampFinger(ptptr, ptr, len1);

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

		len2 = read_stream(fp, buffer, &size);
		fclose(fp);

		if(len1 != len2)
		{
			free(buffer);
			printf("Length dismatch\r\n");
			return -1;
		}

		ptptr = print + FPSize;
		stampFinger(ptptr, ptr, len2);
		for(i=0; i<FPSize; i++)	fprintf(stdout, "%d ", ptptr[i]);
		fprintf(stdout, "\r\n");

		fprintf(stdout, "Esitmate: %d \r\n", estimate(ptptr - FPSize, ptptr, FPSize));
	}

	free(buffer);
	return 0;
}



