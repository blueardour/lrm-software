//  ******************************************************************//
//  author: chenp
//  version: 1.0
//	date: 2014-03-06
//  description: dump sequence at position
//  ******************************************************************//


#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "utils.h"

typedef long FType;

struct Options
{
	unsigned long pos;
	unsigned long len;
	char * database;
};

int print_help(char * s)
{
  printf("Usage(version 1.0):\r\n%s", s);
  printf(" -l[en] num -p[os] num -d[atabase] str\r\n");
  return 0;
}

void init_Options(struct Options * op)
{
  op->len = 1000;
  op->database = NULL;
	op->pos = 0;
}

void format_Options(struct Options * op)
{ 
}

void dump_Options(struct Options * op)
{ 
}

int dump_ex(struct Options * op)
{
	FILE * database;
	char * buffer;

	if(strcmp(getFileType(op->database), "ex") != 0)
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

	buffer = (char *)malloc(op->len+1);
	buffer[op->len]=0;

	if(fseek(database, op->pos, SEEK_SET) != 0)
	{
		printf("Fseek error\r\n");
		return -3;
	}
	if(fread(buffer, 1, op->len, database) != op->len)
		printf("Read file Error(EOF:%d, ERROR:%d)\r\n", ferror(database), feof(database));
	else
		printf("%s\r\n", buffer);

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
  while( (c=getopt(argc,argv,"l:d:p:")) >=0)
  {
    switch(c)
    {
      case 'l':options++; op.len = atoi(optarg); break;
      case 'p':options++; op.pos = atoi(optarg); break;
      case 'd':options++; op.database = optarg; break;
      default: return print_help(getFileName(argv[0]));
    }
  }
  if(options < 3) return print_help(getFileName(argv[0]));

  format_Options(&op);

  return dump_ex(&op);
}



