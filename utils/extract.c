//  ******************************************************************//
//  author: chenp
//  version: 1.0
//  date: 2012-6-27
//	update: 2014-01-17
//  description: extract 'ACTG' charactor from the sequence
//  ******************************************************************//


#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char * getFileName(char * FilePath);

struct Options
{
  char * database;
  char * filename;
};

int print_help(char * function)
{
  printf("Usage(version 1.0):\r\n%s", function);
  printf(" -d[atabase] str -f[ilename] str\r\n");
  return 0;
}

void init_Options(struct Options * op)
{
  op->database = NULL;
  op->filename = NULL;
}

void format_Options(struct Options * op)
{
	int len;
  if(op->filename == NULL)
	{
		len = strlen(getFileName(op->database)) + 1;
		op->filename = (char *) malloc(len);
		strcpy(op->filename, getFileName(op->database));
		op->filename[len-1] = 0;
		op->filename[len-2] = 'x';
		op->filename[len-3] = 'e';
	}
}

char * getFileName(char * FilePath)
{
  char * filename;
  int i;
  filename = FilePath;
  for(i=0; i<strlen(FilePath); i++)
  {
    if(FilePath[i] == '\\' || FilePath[i] == '/')
      filename = FilePath + i + 1;  
  }
  return filename;
}

int read_util(FILE *fp, char c, int forward, FILE * fp2)
{
  char s;
  while((s=fgetc(fp)) != EOF)
  {
	if(s==c) return 0;
	else if(forward != 0) if(s=='A' || s=='C' || s=='G' || s=='T') fputc(s, fp2);
  }
  return -1;
}

int extract(struct Options * op)
{
  FILE * database, * fp;
  int seq;
  char c;

  database = fopen(op->database,"r");
  if(database == NULL)
  {
    printf("Database cannot open:%s\r\n",op->database);
    return -3;
  }
  
  fp = fopen(op->filename,"w");
  if(fp == NULL)
  {
    printf("Can create file:%s\r\n",op->filename);
    return -5;
  }

  seq = 0;
  if(read_util(database, '>', 0, NULL) != 0) return -4;
  while(1) 
  {
		if(read_util(database, '\n', 0, NULL) != 0) break;
		seq++;
		if(read_util(database, '>', 1, fp) != 0) break;
  }

  fclose(fp);
  fclose(database);
  fp = database = NULL;
  return seq;
}

int main(int argc, char ** argv)
{
  struct Options op;
  char c;
  int options;

  init_Options(&op);
  options = 0;
  while( (c=getopt(argc,argv,"d:f:")) >=0)
  {
    switch(c)
    {
      case 'f':op.filename = optarg; break;
      case 'd':options++; op.database = optarg; break;
      default: return print_help(getFileName(argv[0]));
    }
  }
  if(options < 1) return print_help(getFileName(argv[0]));
	format_Options(&op);

  return extract(&op);
}



