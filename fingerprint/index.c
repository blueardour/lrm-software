//  ******************************************************************//
//  author: chenp
//  version: 1.0
//	date: 2014-02-28
//  description: load fingerprint of sequence databse from file
//  ******************************************************************//


#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "utils.h"

typedef long FType;

struct fingerprint
{
	long position;
	FType FA, FC, FG, FT;
	FType BA, BC, BG, BT;
};

struct pt_index
{
	int flag;
	unsigned long dlen;
	FType Fmin, Fmax;
	FType Bmin, Bmax;
	struct fingerprint * database;
};

struct Options
{
	long length;
	long begin, end;
	long interval;
	long number;
	int power;
	char * database;
	struct pt_index * index;
};

int print_help(char * s)
{
  printf("Usage(version 1.0):\r\n%s", s);
  printf(" -d[atabase] str \r\n");
  return 0;
}

void init_Options(struct Options * op)
{
  op->length = 0;
	op->begin = 0;
	op->end = -1;
	op->interval = 1;
  op->database = NULL;
  op->power = 0;
	op->number = 0;
	op->index = NULL;
}

void format_Options(struct Options * op)
{ 
}

void dump_Options(struct Options * op)
{ 
	printf("begin:%ld end:%ld len:%ld int:%ld power:%d num:%ld\r\n", \
				op->begin, op->end, op->length, op->interval, op->power, op->number);
}

int load_index(struct Options * op)
{
	FILE * database;
	unsigned long dlen;
	unsigned long i;
	char * buffer;
	struct fingerprint * ptp;

	if(op->database == NULL) return -1;
	if(strcmp(getFileType(op->database), "pt") != 0)
	{
		printf("Error: database type not support:%s\r\n", getFileType(op->database));
		return -1;
	}

  database = fopen(op->database,"r");
  if(database == NULL)
  {
    printf("Database cannot open:%s\r\n",op->database);
    return -2;
  }

	buffer = (char *)malloc(200);
	memset(buffer, 0, 200);

  fseek(database, 0, SEEK_SET);
	if(fscanf(database, "ref:%s dlen:%ld beg:%ld end:%ld int:%ld len:%ld pow:%d\r\n", \
				buffer, &dlen, &op->begin, &op->end, &op->interval, &op->length, &op->power) < 0)
	{
		printf("Read file head error \r\n");
		return -3;
	}

	if(op->end <= op->begin) op->end = dlen;
	op->number = (op->end - op->length - op->begin) / op->interval;
	if(op->number <= 0)
	{
		printf("Options error. number:%ld\r\n", op->number);
		return -4;
	}

	op->index = (struct pt_index *)malloc(sizeof(struct pt_index));
	op->index->flag = 0;
	op->index->dlen = op->number;
	op->index->Fmax = op->index->Bmax = 0;
	op->index->Fmin = op->index->Bmin = -1;
	op->index->database = (struct fingerprint *)malloc(op->index->dlen * sizeof(struct fingerprint));

	for(i=0,ptp=op->index->database; i<op->index->dlen; i++,ptp++)
	{
		if(fscanf(database, "%ld:%ld %ld %ld %ld - %ld %ld %ld %ld\r\n", &ptp->position, &ptp->FA, \
					&ptp->FC, &ptp->FG, &ptp->FT, &ptp->BA, &ptp->BC, &ptp->BG, &ptp->BT) <0) break;
		op->index->Fmax = max5(op->index->Fmax, ptp->FA, ptp->FC, ptp->FG, ptp->FT);
		op->index->Bmax = max5(op->index->Bmax, ptp->BA, ptp->BC, ptp->BG, ptp->BT);
		op->index->Fmin = min5(op->index->Fmin, ptp->FA, ptp->FC, ptp->FG, ptp->FT);
		op->index->Bmin = min5(op->index->Bmin, ptp->BA, ptp->BC, ptp->BG, ptp->BT);
	}
	
	printf("Warning: %ld fingerprint read while expect %ld\r\n", i, op->index->dlen);

	if((op->index->Fmax - op->index->Fmin) < (1<<16))
		printf("Forward fingerprint can be packed!\r\n");
	else
		printf("Forward fingerprint can't be packed > %ld\r\n",op->index->Fmax - op->index->Fmin);

	if((op->index->Bmax - op->index->Bmin) < (1<<16))
		printf("Backward fingerprint can be packed!\r\n");
	else
		printf("Backward fingerprint can't be packed > %ld\r\n",op->index->Bmax - op->index->Bmin);

	free(op->index->database);
	op->index->database = NULL;
	free(op->index);
	op->index = NULL;
  free(buffer);
  buffer = NULL;
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
  while( (c=getopt(argc,argv,"d:")) >=0)
  {
    switch(c)
    {
      case 'd':options++; op.database = optarg; break;
      default: return print_help(getFileName(argv[0]));
    }
  }
  if(options < 1) return print_help(getFileName(argv[0]));

  format_Options(&op);

  return load_index(&op);
}



