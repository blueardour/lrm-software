//  ******************************************************************//
//  author: chenp
//  version: 1.0
//  date: 2012-6-27
//  description: generate certain amount of reads as the simulated data
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
  printf("Usage(version 1.0):\r\n");
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
  //printf("\r\nFormat Options...\r\n");
  if(op->filename == NULL)
  {
    op->filename = (char *)malloc(200*sizeof(char));
    memset(op->filename,0,200*sizeof(char));
    sprintf(op->filename,"A%d-L%d-E%d-%s",op->amount,op->length,op->error,getFileName(op->database));
  }
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

int generate_queries(struct Options * op)
{
  int i,j;
  FILE * database, * fp;
  unsigned long dlen;
  char *string;
  int count;

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
  //fseek(database,0,SEEK_SET);

  fp = fopen(op->filename,"w");
  if(fp == NULL)
  {
    printf("Can create file:%s\r\n",op->filename);
    return -5;
  }

  string = (char *)malloc(op->length+1);
  for(i=0; i<op->amount; i++)
  {
    srand(stime());
    //printf("Rand:%d\r\n",rand());
    if(fseek(database,rand()%(dlen-op->length*2),SEEK_SET) == -1)
    {
      printf("Fseek Error\r\n");
      return -6;
    }
    for(count=j=0; count<100 && j<op->length; j++)
    {
      string[j]=getc(database);
      if(string[j]=='a' || string[j]== 'c' ||string[j]=='g' || string[j]== 't')
      {
        string[j] = string[j] + 'A' - 'a';
        count = 0;
      }
      else if(string[j]=='A' || string[j]== 'C' ||string[j]=='G' || string[j]== 'T') count = 0;
      else { j--; count++; }
    }
    if(j==op->length) string[j]=0;
    else return -7;
    
    for(j=0; j<op->length * op->error / 100; j++)
    {
      srand(stime());
      count = rand()%op->length;
      if(count%5 == 0)
      {
        string[count]='N';
      }
      else 
      {
         if(string[count] == 'A') string[count]='T';
         else if(string[count] == 'C') string[count]='G';
         else if(string[count] == 'G') string[count]='C';
         else if(string[count] == 'T') string[count]='A';
      } 
    }
    
    for(count=j=0; j<= op->length; j++)
    {
      if(string[j] != 'N') string[count++] = string[j];
      else string[count] = string[j];
    }  

    fprintf(fp,"> %s-%d length:%d\r\n%s\r\n",op->filename,i,count-1,string);
    if(i%1000==0) printf(".");
  }
  free(string);
  string = NULL;
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



