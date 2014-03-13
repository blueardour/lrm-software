//  ******************************************************************//
//  author: chenp
//  description: feature based long read alinger
//  version: 1.0
//	date: 2014-03-10
//  ******************************************************************//


#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "utils.h"

#ifndef VERSION
#define VERSION "1.0"
#endif

#ifndef PROGRAM
#define PROGRAM "fblra"
#endif

typedef u32 FType;

struct Options
{
	u08 verbose;
	u08 er, ep;
	u32 length, interval;
	u32 begin, end;
	char * database;
	char * filename;
};

struct DHeader
{
	u32 beg, end;
};

int print_help()
{
	printf("%s:\r\n", VERSION);
  printf("%s -i(nterval) num -l(ength) num -d[atabase] str | -V\r\n", PROGRAM);
  return 0;
}

void init_Options(struct Options * op)
{
  op->length = 1000;
	op->begin = 0;
	op->end = 0;
	op->interval = 128;
	op->verbose = 0;
  op->database = NULL;
  op->filename = NULL;
}

void format_Options(struct Options * op)
{ 
	int len;
	if(op->interval <= 0) op->interval = 128;
	if(op->begin < 0)	op->begin = 0;
	if(op->filename == NULL && op->verbose == 0)
	{
		len = strlen(getFileName(op->database));
		op->filename = (char *)malloc(len);
		strcpy(op->filename, getFileName(op->database));
		op->filename[len-1] = 0;
		len = len - strlen(getFileType(op->filename));
		op->filename[len] = 0;
	}
}

void dump_Options(struct Options * op)
{ 
	fprintf(stderr, "filename:%s begin:%ld end:%ld \r\n", op->filename, op->begin, op->end);
}

int build_fingerprint(struct Options * op)
{
	u32 i,j;
	u32 dlen;
	FType finger[2][4], print[2][4], * power;
	char * ptr, *buffer[2];
	FILE * database, * fp;
	int tmp;
 
  if(op->length <1 || op->length > 65000) return -1;

	ptr = getFileType(op->database);
	if(strcmp(ptr, "fa") != 0 && strcmp(ptr, "fasta") != 0)
	{
		fprintf(stderr, "Error: database type not support:%s\r\n", ptr);
		return -2;
	}

  database = fopen(op->database,"r");
  if(database == NULL)
  {
    fprintf(stderr, "Database cannot open:%s\r\n",op->database);
    return -2;
  }

	printf("=> Parsing database...\r\n");
  fseek(database,0,SEEK_END);

	if(op->end <= op->begin) op->end = dlen;

	fp = NULL;
	if(op->verbose == -1)
	{
		fp = fopen(op->filename,"w");
		if(fp == NULL)
		{
			printf("Can create file:%s\r\n",op->filename);
			return -2;
		}
	}

	buffer = (char *)malloc(op->length+1);
	rev_buffer = (char *)malloc(op->length+1);
	buffer[op->length]=0;
	rev_buffer[op->length]=0;

  power = (FType *)malloc(sizeof(FType)*op->length);
	memset(power, 0, sizeof(FType)*op->length);
	switch(op->power)
	{
		case 0: for(i=0; i<op->length; i++) power[i] = op->length - i; break;
		case 1: for(i=0; i<op->length; i++) power[i] = i; break;
		case 2: for(i=0; i<op->length; i++) power[i] = 1; break;
		case 3: for(i=0; i<op->length; i++) power[i] = op->length - i; break;
	}
	
	if(op->type == 0)
	{
		if(op->verbose == -1) fprintf(fp, "ref:%s dlen:%ld beg:%ld end:%ld int:%ld len:%ld pow:%d\r\n", \
					getFileName(op->database), dlen, op->begin, op->end, op->interval, op->length, op->power);
		else printf("ref:%s dlen:%ld beg:%ld end:%ld int:%ld len:%ld pow:%d\r\n", \
					getFileName(op->database), dlen, op->begin, op->end, op->interval, op->length, op->power);

		for(i=op->begin; i<(op->end-op->length); i+=op->interval)
		{
			fseek(database, i, SEEK_SET);
			tmp = fread(buffer, 1, op->length, database);
			if(tmp != op->length)
			{
				printf("Read file Error(pos:%ld, %d bytes readed) (EOF:%d, ERROR:%d)\r\n", \
								i, tmp, ferror(database), feof(database));
				break;
			}

			for(j=0; j<op->length; j++)
				rev_buffer[j] = buffer[op->length-1-j];

			memset(finger, 0, sizeof(FType) * 4);
			memset(print , 0, sizeof(FType) * 4);
			memset(rev_finger, 0, sizeof(FType) * 4);
			memset(rev_print , 0, sizeof(FType) * 4);
			for(j=0; j<op->length; j++)
			{
				switch(buffer[j])
				{
					case 'A': finger[0]++; break;
					case 'C': finger[1]++; break;
					case 'G': finger[2]++; break;
					case 'T': finger[3]++; break;
					default: printf("Unexpected char:%d \r\n", buffer[j]); return -3;
				}
				print[0] += finger[0]*power[j];
				print[1] += finger[1]*power[j];
				print[2] += finger[2]*power[j];
				print[3] += finger[3]*power[j];

				switch(rev_buffer[j])
				{
					case 'A': rev_finger[0]++; break;
					case 'C': rev_finger[1]++; break;
					case 'G': rev_finger[2]++; break;
					case 'T': rev_finger[3]++; break;
					default: printf("Unexpected char:%d \r\n", rev_buffer[j]); return -3;
				}
				rev_print[0] += rev_finger[0]*power[j];
				rev_print[1] += rev_finger[1]*power[j];
				rev_print[2] += rev_finger[2]*power[j];
				rev_print[3] += rev_finger[3]*power[j];
			}

			if(op->verbose == -1) 
				fprintf(fp, "%ld:%ld %ld %ld %ld - %ld %ld %ld %ld\r\n", i, print[0], print[1], \
								print[2], print[3], rev_print[0], rev_print[1], rev_print[2], rev_print[3]);
			else
			{
				for(j=0; j<op->length; j++)
					fprintf(stderr, "%c\r\n", buffer[j]);
				printf("%ld:%ld %ld %ld %ld - %ld %ld %ld %ld\r\n", i, print[0], print[1], \
								print[2], print[3], rev_print[0], rev_print[1], rev_print[2], rev_print[3]);
			}
		}
	}
	else
	{
		fseek(database, 0, SEEK_SET);
		if(read2b_util(database, '>', 0, NULL, 0) != 0) return -4;

		//printf("verbose:%d fp:%0x\r\n", op->verbose, fp);
		while(1)
		{
			if(op->verbose == 1)
			{
				if(read2s_util(database, '\n', 2) != 0) break;
			}
			else 
			{
				if(read2f_util(database, '\n', 2, fp) != 0) break;
			}

			if(read2b_util(database, '>', 1, buffer, op->length) == -2) break;

			memset(finger, 0, sizeof(FType) * 4);
			memset(print , 0, sizeof(FType) * 4);
			for(j=0; j<op->length; j++)
			{
				switch(buffer[j])
				{
					case 'A': finger[0]++; break;
					case 'C': finger[1]++; break;
					case 'G': finger[2]++; break;
					case 'T': finger[3]++; break;
					default: printf("Unexpected char:%d \r\n", buffer[j]); return -3;
				}
				print[0] += finger[0]*power[j];
				print[1] += finger[1]*power[j];
				print[2] += finger[2]*power[j];
				print[3] += finger[3]*power[j];
			}

			if(op->verbose == -1) 
				fprintf(fp, "%ld %ld %ld %ld\r\n", print[0], print[1], print[2], print[3]);
			else if(op->verbose-- == 1)
			{
				for(j=0; j<op->length; j++)
					fprintf(stderr, "%c\r\n", buffer[j]);
				printf("%ld %ld %ld %ld\r\n", print[0], print[1], print[2], print[3]);
				break;
			}
		}
	}

  free(buffer);
  free(rev_buffer);
  buffer = rev_buffer = NULL;
  if(fp != NULL) fclose(fp);
  fclose(database);
  fp = database = NULL;
  return 0;
}

int main(int argc, char ** argv)
{
  struct Options op;
  char c;
  int options;

  init_Options(&op);
  options = 0;
  while( (c=getopt(argc,argv,"l:b:d:i:f:e:v:")) >=0)
  {
    switch(c)
    {
      case 'b':op.begin = atoi(optarg); break;
      case 'e':op.end = atoi(optarg); break;
      case 'l':options++; op.length = atoi(optarg); break;
      case 'i':op.interval = atoi(optarg); break;
      case 'f':op.filename = optarg; break;
      case 'd':options++; op.database = optarg; break;
      case 'v': op.verbose = atoi(optarg); break;
      default: return print_help(getFileName(argv[0]));
    }
  }
  if(options < 2) return print_help(getFileName(argv[0]));

  format_Options(&op);

  return build_fingerprint(&op);
}



