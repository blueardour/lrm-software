//  ******************************************************************//
//  author: chenp
//  version: 1.0
//  date: 2014-01-16
//  ******************************************************************//


#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct Options
{
  char * database;
  char * filename;
};

int print_help(char * function)
{
  printf("Usage(version 1.0):\r\n%s", function);
  printf(" -d[atabase] str -f pac\r\n");
  return 0;
}

void init_Options(struct Options * op)
{
  op->database = op->filename = NULL;
}

void format_Options(struct Options * op)
{
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

unsigned char table[256] = {
	4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
	4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
	4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 5, 4, 4,
	4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
	4, 0, 4, 1,  4, 4, 4, 2,  4, 4, 4, 4,  4, 4, 4, 4, 
	4, 4, 4, 4,  3, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
	4, 0, 4, 1,  4, 4, 4, 2,  4, 4, 4, 4,  4, 4, 4, 4, 
	4, 4, 4, 4,  3, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
	4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
	4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
	4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
	4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
	4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
	4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
	4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
	4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4
};

char pack_byte(char c1, char c2, char c3, char c4)
{
  char buffer = 0;
  buffer = table[c4] | (table[c3] << 2) | (table[c2] << 4) | (table[c1] << 6);
  return buffer;
}

int pack_bytes(struct Options * op)
{
  FILE * database, * fp;
  unsigned long dlen, plen;
  unsigned long i;
  int j;
  char seq[4];
  char c;

  database = fopen(op->database,"r");
  if(database == NULL)
  {
    printf("Database cannot open:%s\r\n",op->database);
    return -1;
  }
  fseek(database, 0, SEEK_END);
  dlen = ftell(database);
  fseek(database, -dlen, SEEK_END);
  if(ftell(database) != 0)
  {
	printf("File seek failed \r\n");
	return -3;
  } 

  fp = fopen(op->filename,"wb");
  if(fp == NULL)
  {
    printf("Can create file:%s\r\n",op->filename);
    return -2;
  }

  plen = (dlen%4 == 0) ? dlen : (dlen-dlen%4);
  for(i=0; i<plen; i+=4)
  {
	c = pack_byte(fgetc(database),fgetc(database),fgetc(database),fgetc(database));
	fputc(c, fp);
  }

  memset(seq, 0, 4);
  if(dlen != plen)
  {
	for(j=0; j<dlen%4; j++) seq[j]=fgetc(database);
	c = pack_byte(seq[0],seq[1],seq[2],seq[3]);
	fputc(c, fp);
  }
  else fputc(0, fp);

  fputc(dlen%4, fp);

  fclose(fp);
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
  while( (c=getopt(argc,argv,"d:f:")) >=0)
  {
    switch(c)
    {
      case 'd':options++; op.database = optarg; break;
      case 'f':options++; op.filename = optarg; break;
      default: return print_help(argv[0]);
    }
  }
  if(options != 2) return print_help(argv[0]);
  format_Options(&op);
  return pack_bytes(&op);
}



