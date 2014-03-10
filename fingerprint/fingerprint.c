//  ******************************************************************//
//  author: chenp
//  version: 1.0
//	date: 2014-02-24
//  description: build fingerprint of sequence 
//  ******************************************************************//


#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "utils.h"

typedef long FType;

struct Options
{
	unsigned long length;
	unsigned long begin, end;
	unsigned long interval;
	int type;  // 0 indicate reference, others indicate query
	int power;
	int verbose;
	char * database;
	char * filename;
};

int print_help(char * s)
{
  printf("Usage(version 1.0):\r\n%s", s);
  printf(" -b[egin] num -e[nd] num -i[nterval] num -l[ength] num -d[atabase] str -f[ilename] str -v num\r\n");
  printf("(begin = 0, end = SEEK_END, interval = 10 if not specific.)\r\n");
  return 0;
}

void init_Options(struct Options * op)
{
  op->length = 1000;
	op->begin = 0;
	op->end = 0;
	op->interval = 10;
  op->database = NULL;
  op->filename = NULL;
	op->type = 0;
	op->power = 2;
	op->verbose = -1;
}

void format_Options(struct Options * op)
{ 
	int len;
	if(op->interval <= 0) op->interval = 10;
	if(op->begin < 0)
	{
		printf("Warning: '-b num' now is %ld\r\n", op->begin);
		op->begin = 0;
	}
	if(op->end < 0) op->end = -1;

	if(op->filename == NULL && op->verbose == -1)
	{
		len = strlen(getFileName(op->database)) + 1;
		op->filename = (char *)malloc(len);
		strcpy(op->filename, getFileName(op->database));
		op->filename[len-1] = 0;
		op->filename[len-2] = 't';
		op->filename[len-3] = 'p';
	}

	if(strcmp(getFileType(op->database), "ex") == 0) op->type = 0;
	else op->type = 1;

	if(op->verbose == 0) op->verbose = 1; 
}

void dump_Options(struct Options * op)
{ 
	printf("begin:%ld end:%ld \r\n",op->begin, op->end);
}

int build_fingerprint(struct Options * op)
{
	FILE * database, * fp;
	unsigned long i,j;
	unsigned long dlen;
	FType finger[4], print[4];
	FType rev_finger[4], rev_print[4];
	FType * power;
	char * buffer, *rev_buffer;
	int tmp;

	if(strcmp(getFileType(op->database), "ex") == 0)
	{
		printf("database seems as a reference, set type to zero\r\n");
		op->type = 0;
	}
	else if(strcmp(getFileType(op->database), "fa") == 0)
	{
		printf("database seems as a query, set type to non-zero\r\n");
		op->type = 1;
	}
	else
	{
		printf("Error: database type not support:%s\r\n", getFileType(op->database));
		return -1;
	}

  if(op->length <1 || op->length > 65000) return -1;
  database = fopen(op->database,"r");
  if(database == NULL)
  {
    printf("Database cannot open:%s\r\n",op->database);
    return -2;
  }

  fseek(database,0,SEEK_END);
  dlen = ftell(database);
  if(dlen < op->length)
  {
    fclose(database); database=NULL;
    printf("Database length (%ld) is too short\r\n",dlen);
    return -2;
  }
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



