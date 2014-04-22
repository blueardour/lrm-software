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

	while(c=fgetc(fp), c != EOF && c != '>')
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
	char * buffer, * ptr;
	int len1, size;

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
	fprintf(stdout, "Score: %d\r\n", SmithWaterman(ptr, len1));

	free(buffer);
	return 0;
}



