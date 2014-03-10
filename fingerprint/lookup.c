//  ******************************************************************//
//  author: chenp
//  version: 1.0
//	date: 2014-02-26
//  description: lookup given fingerprint in a reference fingerprint database
//  ******************************************************************//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "utils.h"

char * getFileName(char * FilePath);
char * getFileType(char * FilePath);

typedef unsigned int FType;

struct Options
{
	int max;
	int items;
	FType A, C, G, T;	
  char * database;
};

int print_help(char * s)
{
  printf("Usage(version 1.0):\r\n%s", s);
  printf(" -m[ax] num -i[tems] num -d[atabase] str \r\n");
  return 0;
}

void init_Options(struct Options * op)
{
  op->database = NULL;
	op->max = 50000;
	op->items = 10;
	op->A = op->C = op->G = op->T = 0;
}

void format_Options(struct Options * op)
{ 
}

void dump_Options(struct Options * op)
{ 
	printf("Max:%d Items:%d A:%d C:%d G:%d T:%d \r\n", op->max, \
					op->items, op->A, op->C, op->G, op->T);
}

struct pos_entry
{
	unsigned int pos;
	unsigned int sum;
};

int lookup_fingerprint(struct Options * op)
{
  FILE * database;
  char * buffer;
	size_t tmp;
	int index;
	unsigned int position, sum;
	struct pos_entry * pos;
	FType print[4];

	if(strcmp(getFileType(op->database), "pt") != 0)
	{
		printf("Input file seems not to be a fingerprint database \r\n");
		return -1;
	}

  database = fopen(op->database,"r");
  if(database == NULL)
  {
    printf("Database cannot open:%s\r\n",op->database);
    return -2;
  }
	
	print[0] = print[1] = print[2] = print[3] = 0;
	tmp = 200;
	buffer =(char *)malloc(tmp);
	memset(buffer, 0, tmp);
	if(getline(&buffer, &tmp, database) < 0)
	{
		printf("Read file error-1\r\n");
		return -2;
	}

	pos = (struct pos_entry *)malloc(op->items * sizeof(struct pos_entry));
	index = 0;

	for(index=0; index<op->items; index++)
	{
		if(fscanf(database, "%d:%d,%d,%d,%d\r\n", \
				&position, print, print+1, print+2, print+3) < 0)
			return -3;
		pos[index].pos = position;
		pos[index].sum = diff(print[0], op->A) + diff(print[1], op->C) + \
								diff(print[2], op->G) + diff(print[3], op->T);
	}

	do {
		if(fscanf(database, "%d:%d,%d,%d,%d\r\n", \
				&position, print, print+1, print+2, print+3) < 0)
			break;

		for(index=0; index<op->items; index++)
		{
			sum = diff(print[0], op->A) + diff(print[1], op->C) + \
				diff(print[2], op->G) + diff(print[3], op->T);
			if(sum <= pos[index].sum && sum <= op->max)
			{
				pos[index].pos = position;
				pos[index].sum = sum;
				break;
			}
		}
	} while(1);

	// dump results
	for(index=0; index<op->items; index++)
	{
		printf("pos:%d, sum:%d \r\n", pos[index].pos, pos[index].sum);
	}

  fclose(database);
  database = NULL;
  return 0;
}

int main(int argc, char ** argv)
{
  struct Options op;
  char c;
  int options;
  init_Options(&op);
  options = 0;
  while( (c=getopt(argc,argv,"m:i:d:")) >=0)
  {
    switch(c)
    {
      case 'm':op.max = atoi(optarg); break;
      case 'i':op.items = atoi(optarg); break;
      case 'd':options++; op.database = optarg; break;
      default: return print_help(getFileName(argv[0]));
    }
  }

  if(options < 1) return print_help(getFileName(argv[0]));

	printf("argc:%d optind:%d \r\n", argc, optind);
	if((argc - optind) == 4)
	{
		op.A = atoi(argv[optind]);
		op.C = atoi(argv[optind+1]);
		op.G = atoi(argv[optind+2]);
		op.T = atoi(argv[optind+3]);
	}
	else
	{
		printf("Wrong number of fingerprint\r\n");
		return -1;
	}
		
  format_Options(&op);
	dump_Options(&op);

  return lookup_fingerprint(&op);
}



