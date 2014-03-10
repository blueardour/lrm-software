//  ******************************************************************//
//  author: chenp
//  version: 1.1
//  date: 2012-8-7
//	update 2014-01-17
//  description: census alignment result
//  ******************************************************************//


#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct Options
{
  char * bitwise;
  char * quality;
};

int print_help(char *s)
{
  printf("Usage:\r\n%s", s);
  printf(" -b[itwise] file1 -q[uality] file2 \r\n");
  return 0;
}

void init_Options(struct Options * op)
{
  op->bitwise = NULL;
  op->quality = NULL;
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

void format_Options(struct Options * op)
{ 
  //printf("\r\nFormat Options...\r\n");
  //printf("Options:\r\n[%d]-[%d]-[%d]-[%s]-[%s]\r\n",op->length,op->amount,op->error, \
	op->database,op->filename);

}

unsigned int stime()
{
  static unsigned int ctime = 0;
  struct timeval start;

  gettimeofday(&start, NULL);
  ctime = start.tv_sec + start.tv_usec * 1000 - ctime;
  return ctime;
}

int census(struct Options * op)
{
  int i,j;
  FILE * fp;
  int data;

  int count,total;
  int n0,n1,n2,n4,n8,n16,no;

  count = total = 0;
  if(op->quality != NULL) 
  {
    fp = fopen(op->quality,"r");
    if(fp == NULL)
    {
      return -3;
    }
    while(fscanf(fp,"%d",&data) != EOF)
    {
      if(data>=20) count++;
      total++;
    }
    printf("\r\n*** Quality ***\r\n");
    printf("Total:%d Count:%d Low:%d Q20:%.2f\r\n",total,count,total-count,count*100.0/total);
    fclose(fp);
    fp = NULL;
  }

  //printf("/*******************/\r\n");

  n0 = n1 = n2 = n4 = n8 = n16 = no = 0;
  if(op->bitwise != NULL)
  {
    fp = fopen(op->bitwise,"r");
    if(fp == NULL)
    {
      return -4;
    }
    while(fscanf(fp,"%d",&data) != EOF)
    {
      switch(data)
      {
        case 0: n0++; break;
        case 1: n1++; break;
        case 2: n2++; break;
        case 4: n4++; break;
        case 8: n8++; break;
        case 16:n16++; break;
        default: no++; break;
      }
    }
    printf("\r\n*** Bitwise ***\r\n");
    printf("N0\tN1\tN2\tN4\tN8\tN16\tNO\r\n");
    printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\r\n",n0,n1,n2,n4,n8,n16,no);
    fclose(fp);
    fp = NULL;
  }

  if(total != count)
  {
    printf("\r\n*** Addition ***\r\n");
    printf("Unmap\tReserve\r\n");
    printf("%.2f\t%.2f\r\n",n4*100.0/total,n16*100.0/total);
  }
  return 0;
}

int main(int argc, char ** argv)
{
  struct Options op;
  char c;
  int options;

  init_Options(&op);
  options = 0;
  while( (c=getopt(argc,argv,"b:q:")) >=0)
  {
    switch(c)
    {
      case 'b':options++; op.bitwise = optarg; break;
      case 'q':options++; op.quality = optarg; break;
      default: return print_help(argv[0]);
    }
  }
  if(options == 0) return print_help(argv[0]);
  return census(&op);
}



