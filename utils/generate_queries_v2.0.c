//  ******************************************************************//
//  author: chenp
//  version: 1.0
//  date: 2012-6-27
//  description: generate certain amount of reads as the simulated data
//  bug fix:
//  2013.12.30 add random additions and deletations or variations in the query
//  2013.12.31 delete the space just after '>'
//  
//  ******************************************************************//


#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct Options
{
  int length;
  int error;   // percent
  int amount;
  char * database;  // filename
  char * filename;
};

int print_help()
{
  printf("Usage(version 2.0):\r\n");
  printf("  -a[mount] num -l[ength] num -e[rror] num -d[atabase] str -f[ilename] str\r\n");
  return 0;
}

void init_Options(struct Options * op)
{
  op->length = 1000;
  op->error = 3;
  op->amount = 10000;
  op->database = NULL;
  op->filename = NULL;
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
  if(op->filename == NULL)
  {
    op->filename = (char *)malloc(200*sizeof(char));
    memset(op->filename,0,200*sizeof(char));
    sprintf(op->filename,"A%d-L%d-E%d-%s",op->amount,op->length,op->error,getFileName(op->database));
  }
}

unsigned int stime()
{
  static unsigned int ctime = 0;
  struct timeval start;

  gettimeofday(&start, NULL);
  ctime = start.tv_sec + start.tv_usec * 1000 - ctime;
  return ctime;
}

typedef struct Locations_Map
{
  unsigned long pos;
  char c;
} LM;

int generate_queries(struct Options * op)
{
  FILE * database, * fp;
  unsigned long i,j;
  unsigned long dlen,number;
  unsigned long count;
  char *string, *string2;
  char c;
  LM * modify;

  if(op->amount <1 || op->length <1) return -2;
  database = fopen(op->database,"r");
  if(database == NULL)
  {
    printf("Database cannot open:%s\r\n",op->database);
    return -3;
  }
  fseek(database,0,SEEK_END);
  dlen = ftell(database);
  if(dlen < 10000 || dlen < op->length*2)
  {
    fclose(database); database=NULL;
    printf("Database length (%ld) is too short\r\n",dlen);
    return -4;
  }
  
  number = dlen * op->error * 4 / 100 /5;
  modify = (LM *)malloc(number * sizeof(LM));
  for(i=0; i<number; i++)
  {
    srand(stime());
    count = rand()%(dlen-op->length);
    if(count%4 == 0) { modify[i].pos = count; modify[i].c = 'T'; }
    if(count%4 == 1) { modify[i].pos = count; modify[i].c = 'G'; }
    if(count%4 == 2) { modify[i].pos = count; modify[i].c = 'C'; }
    if(count%4 == 3) { modify[i].pos = count; modify[i].c = 'A'; }
  }


  fp = fopen(op->filename,"w");
  if(fp == NULL)
  {
    printf("Can create file:%s\r\n",op->filename);
    return -5;
  }

  string = (char *)malloc(op->length+1);
  string2 = (char*)malloc(op->length*2+1);
  for(i=0; i<op->amount; i++)
  {
    srand(stime());
    count = rand()%(dlen-op->length*2);
    if(fseek(database,count,SEEK_SET) == -1)
    {
      printf("Fseek Error\r\n");
      return -6;
    }
    for(j=0; j<op->length; j++)
    {
      c=getc(database);
      if(c=='a' || c== 'c' || c=='g' || c== 't')
      {
        string[j] = c + 'A' - 'a';
      }
      else if(c=='A' || c== 'C' || c=='G' || c== 'T') string[j] = c;
      else j--;
    }
    if(j==op->length) string[j]=0;
    else return -7;

    for(j=0; j<number; j++)
    {
      if(modify[j].pos >= count && modify[j].pos < count+op->length)
        string[modify[j].pos-count] = modify[j].c;
    }
    
    for(j=0; j<op->length*op->error/100/5; j++)
    {
      srand(stime());
      count = rand()%op->length;
      if(count%2) string[count]='D';
      else string[count]= string[count] + 1;
    }

    for(count=j=0; j<op->length;)
    {
      if(string[j] == 'D')
      {
        string2[count] = string[j++];
        continue;
      }
      if(string[j]=='A'||string[j]=='C'||string[j]=='G'||string[j]=='T')
      {
        string2[count++] = string[j++];
        continue;
      }
      if(string[j]==('A'+1)||string[j]==('C'+1)||string[j]==('G'+1)||string[j]==('T'+1))
      {
        string2[count++] = string[j]-1;
        string2[count++] = string[j++] -1;
        continue;
      }
      printf("Error Char!\r\n");
      return -8;
    }
    string2[count]=0;
    fprintf(fp,">%s-%ld length:%ld\r\n%s\r\n",op->filename,i,count,string2);
  }
  free(string);
  free(string2);
  string2 = NULL;
  string = NULL;
  free(modify);
  modify=NULL;
  fclose(fp);
  fclose(database);
  fp = database = NULL;
  return 0;
}

int main(int argc, char ** argv)
{
  struct Options op;
  char c;
  int options_number;

  init_Options(&op);
  options_number = 0;
  while( (c=getopt(argc,argv,"l:a:d:e:f:")) >=0)
  {
    switch(c)
    {
      case 'l':options_number++; op.length = atoi(optarg); break;
      case 'a':options_number++; op.amount = atoi(optarg); break;
      case 'e':options_number++; op.error  = atoi(optarg); break;
      case 'f':op.filename = optarg; break;
      case 'd':options_number++; op.database = optarg; break;
      default: return print_help();
    }
  }
  if(options_number != 4) return print_help();
  format_Options(&op);
  return generate_queries(&op);
}



