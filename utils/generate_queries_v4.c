//  ******************************************************************//
//  author: chenp
//  version: 4.1
//  date: 2012-6-27
//	update: 2014-01-17
//	update: 2014-02-24 with postion information
//	update: 2014-02-25, change gaps insertion strategyy 
//	update: 2014-03-06, change query name format
//  description: generate certain amount of reads as the simulated data
//  ******************************************************************//


#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char * getFileName(char * FilePath);

struct Options
{
  int length;
  int error;   // percent
  int amount;
	int variation;		// 0:variation and indles; 1:variation only
  char * database;  // filename
  char * filename;
};

int print_help(char * s)
{
  printf("Usage(version 4.0):\r\n%s",s);
  printf(" -v[ariation] -a[mount] num -l[ength] num -e[rror] num -d[atabase] str -f[ilename] str\r\n");
  return 0;
}

void init_Options(struct Options * op)
{
  op->length = 1000;
  op->error = 3;
  op->amount = 10000;
  op->database = NULL;
  op->filename = NULL;
	op->variation = 0;
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

char * getFileType(char * FilePath)
{
	char * type;
	int i;
	type = FilePath;
	for(i=strlen(FilePath)-1; i>=0; i--)
	{
		if(FilePath[i] == '.' )
		{
			type = FilePath + i + 1;
			break;
		}
	}
	return type;
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
  printf("A%d-L%d-E%d-D(%s)\r\n",op->amount,op->length,op->error,op->database);
}

unsigned int stime()
{
  static unsigned int ctime = 0;
  struct timeval start;

  gettimeofday(&start, NULL);
  ctime = start.tv_sec + start.tv_usec * 1000 - ctime;
  return ctime;
}

struct Indels
{
	int pos;
	int len;
	int type;
};

int generate_queries(struct Options * op)
{
  FILE * database, * fp;
  unsigned long i,j,k;
  unsigned long dlen, position, number;
  unsigned long count;
	struct Indels * gaps;
  char *string, *buffer;
  char c;

	if(strcmp(getFileType(op->database), "ex") != 0)
	{
		printf("'ex' file is needed for the reference(%s)\r\n", getFileType(op->database));
		return -3;
	}

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

  fp = fopen(op->filename,"w");
  if(fp == NULL)
  {
    printf("Can create file:%s\r\n",op->filename);
    return -3;
  }

	number = op->length+op->length*op->error/100;
  string = (char *)malloc(op->length);
  buffer = (char *)malloc(op->length+number);
	gaps = (struct Indels *) malloc(number/5*sizeof(struct Indels));

  for(i=0; i<op->amount; i++)
  {
		srand(stime());
		position = rand()%(dlen-op->length-op->length/100);
		if(fseek(database,position,SEEK_SET) == -1)
		{
			printf("Fseek Error\r\n");
			return -4;
		}

		for(j=0; j<op->length; j++)
		{
			c=getc(database);
			if(c=='A' || c== 'C' || c=='G' || c== 'T') string[j] = c;
			else
			{
				printf("Unexpected char:%d at pos:%ld\r\n", c, position+j);
				return -3;
			}
		}

		if(op->variation == 0) //80% in errors are varations
			number = op->length*op->error*4/100/5;
		else
			number = op->length*op->error/100;

		for(j=0; j<number; j++)
		{
			srand(stime());
			count = rand()%op->length;
			if(count%4 == 0) c = 'T';
			if(count%4 == 1) c = 'G';
			if(count%4 == 2) c = 'C';
			if(count%4 == 3) c = 'A';
			if(count>=0 && count<op->length) string[count] = c;
		}

		if(op->variation == 0) // 20% in errors are indels errors: additions or deletions
			number = op->length*op->error/100/5;
		else
			number = 0;

		for(j=0; j<number; j++)
		{
			srand(stime());
			gaps[j].pos = rand()% op->length; // gap position

			gaps[j].len = 1;  // gap len
			gaps[j].type = 0; // addition:non-zero, deletion:zero
			do {
				srand(stime());
				count = rand()%20;
				if(count<=6 && count>=0)
				{
					break;
				}
				else if(count>=7 && count <= 13)
				{
					gaps[j].type = ! gaps[j].type;
					break;
				}
				gaps[j].len++;
				if(gaps[j].len > number)
				{
					gaps[j].type = 0;
					break;
				}
				else gaps[j].type = ! gaps[j].type;
			} while(1);
		}

		// final string 
		for(count=j=0; j<op->length;)
		{
			for(k=0;k<number;k++)
			{
				if(gaps[k].pos == j)
				{
					if(gaps[k].type == 0)
						j+=gaps[k].len;
					else
					{
						while(gaps[k].len--) buffer[count++] = string[op->length-1-gaps[k].len];
						buffer[count++] = string[j++];
					}
					break;
				}
			}

			if(k == number) buffer[count++] = string[j++];
		}
		buffer[count] = 0;

    //printf("%d-%d\r\n",count,j);
    //fprintf(fp,"> %s-%ld length:%ld\r\n%s\r\n",op->filename,i,count,buffer);
    //fprintf(fp,">name(%s)-index[%ld] position:%ld-err:%d length:%ld\r\n%s\r\n", getFileName(op->database),i,position,op->error,count,buffer);
		fprintf(fp,">%s.[%ld].[%ld].[%d] length:%ld\r\n%s\r\n", getFileName(op->database),i,position,op->error,count,buffer);
		
  }

  free(string);
  free(buffer);
	free(gaps);
  fclose(fp);
  fclose(database);
  gaps = NULL;
	string = buffer = NULL;
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
  while( (c=getopt(argc,argv,"l:a:d:e:f:v")) >=0)
  {
    switch(c)
    {
      case 'l':options++; op.length = atoi(optarg); break;
      case 'a':options++; op.amount = atoi(optarg); break;
      case 'e':options++; op.error  = atoi(optarg); break;
      case 'f':op.filename = optarg; break;
      case 'd':options++; op.database = optarg; break;
			case 'v':op.variation = 1; break;
      default: return print_help(getFileName(argv[0]));
    }
  }
  if(options < 4) return print_help(getFileName(argv[0]));
  format_Options(&op);
  //dump_Options(&op);
  return generate_queries(&op);
}



