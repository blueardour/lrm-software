//  ******************************************************************//
//  author: chenp
//  version: 3.0
//  date: 2012-6-27
//	update: 2014-01-17
//  description: generate certain amount of reads as the simulated data
//  ******************************************************************//


#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char * InFileType = "ex";
char * OutFileType= "qu";

struct Options
{
  int length;
  int error;   // percent
  int amount;
  char * database;  // filename
  char * filename;
};

int print_help(char * s)
{
  printf("Usage(version 3.0):\r\n%s",s);
  printf(" -a[mount] num -l[ength] num -e[rror] num -d[atabase] str -f[ilename] str\r\n");
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

void dump_Options(struct Options * op)
{ 
  printf("A%d-L%d-E%d-D%s\r\n",op->amount,op->length,op->error,op->database);
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
  FILE * database, * fp;
  unsigned long i,j,k;
  unsigned long dlen, qlen;
  unsigned long count;
  char *string, *buffer;
  char c;

  if(op->amount <1 || op->length <1) return -2;
  database = fopen(op->database,"r");
  if(database == NULL)
  {
    printf("Database cannot open:%s\r\n",op->database);
    return -1;
  }
  fseek(database,0,SEEK_END);
  dlen = ftell(database);
  if(dlen < 1000 || dlen < op->length*2)
  {
    fclose(database); database=NULL;
    printf("Database length (%ld) is too short\r\n",dlen);
    return -2;
  }

	// file type
	fseek(database, 0, SEEK_SET);
	if(fgetc(database)!=InFileType[0] || fgetc(database)!=InFileType[1])
	{
		printf("Error In File Type \r\n");
		return -5;
	}
  
  fp = fopen(op->filename,"w");
  if(fp == NULL)
  {
    printf("Can create file:%s\r\n",op->filename);
    return -3;
  }

  string = (char *)malloc(op->length*2+1);
  buffer = (char *)malloc(op->length*2+1);

  for(i=0; i<op->amount; i++)
  {
    srand(stime());
    count = rand()%(dlen-op->length*2);
    if(fseek(database,count,SEEK_SET) == -1)
    {
      printf("Fseek Error\r\n");
      return -4;
    }

	// random indels length, addition here
	srand(stime());
	count = rand()%(op->length*op->error/100);
	if(op->error>1) count = count /(op->error-1);
	qlen = op->length + count;
	for(j=0; j<qlen; j++)
	{
		 c=getc(database);
		 if(c==EOF)
		 {
			printf("error reading file! \r\n"); return -5;
		 }
		 else if(c=='A' || c== 'C' || c=='G' || c== 'T') string[j] = c;
		 else // give up this time
		 {
			j--;
			i--;
			break;
			continue;
		}
	}
	//string[j]=0;

    // 80% in errors are varations
    for(j=0; j< op->length*op->error*4/100/5; j++)
    {
      srand(stime());
      count = rand()%(qlen);
      if(count%4 == 0) c = 'T';
      if(count%4 == 1) c = 'G';
      if(count%4 == 2) c = 'C';
      if(count%4 == 3) c = 'A';
			if(count>=0 && count<qlen) string[count] = c;
    }

		// 20% in errors are indels errors, add deletion here
    for(j=0; j<op->length*op->error/100/5; j++)
    {
      srand(stime());
      count = rand()%(qlen);

			srand(stime());
			k=rand()%(op->length*op->error/100)/(op->error+1);
			while(k-->0) if((count+k)<qlen) string[count+k]='D';
		}

		// final string
    for(count=j=0; j<qlen;)
    {
      if(string[j] == 'D')
      {
        buffer[count] = string[j++];
      }
			else if(string[j]=='A'||string[j]=='C'||string[j]=='G'||string[j]=='T')
      {
        buffer[count++] = string[j++];
      }
    }
    buffer[count]=0;
    //printf("%d-%d\r\n",count,j);
    fprintf(fp,"> %s-%ld length:%ld\r\n%s\r\n",op->filename,i,count,buffer);
  }

  free(string);
  free(buffer);
  string = buffer = NULL;
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
  while( (c=getopt(argc,argv,"l:a:d:e:f:")) >=0)
  {
    switch(c)
    {
      case 'l':options++; op.length = atoi(optarg); break;
      case 'a':options++; op.amount = atoi(optarg); break;
      case 'e':options++; op.error  = atoi(optarg); break;
      case 'f':op.filename = optarg; break;
      case 'd':options++; op.database = optarg; break;
      default: return print_help(argv[0]);
    }
  }
  if(options != 4) return print_help(argv[0]);
  format_Options(&op);
  //dump_Options(&op);
  return generate_queries(&op);
}



