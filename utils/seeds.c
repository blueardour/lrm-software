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
	int m, n;
	int size;
	char * database;
	char * filename;
	int debug;
};

struct entry
{
	int len;
	int size;
	long * pos;
};

struct CALs
{
	long addr;
	struct entry * cal;
};


int print_help(char * function)
{
  printf("Usage(version 1.0):\r\n%s", function);
  printf(" -m num -n num -s[ize] num -D[ebug] -d[atabase] str -f cal\r\n");
  return 0;
}

void init_Options(struct Options * op)
{
  op->m = 20;
  op->n = 20;
  op->size = 0;
  op->database = op->filename = NULL;
	op->debug = 0;
}

void dump_Options(struct Options * op)
{
	printf("Dump options:\r\n");
	printf("	m=%d,n=%d,s=%d,database=%s,file=%s\r\n", op->m, op->n, op->size, op->database, op->filename);
}

void format_Options(struct Options * op)
{
  op->m = op->m/4*4;
  op->n = op->n/4*4;
}

int read_util(FILE *fp, char c, int forward, FILE * fp2)
{
  char s;
  while((s=fgetc(fp)) != EOF)
  {
    if(s==c) return 0;
    else if(forward != 0) if(s=='A' || s=='C' || s=='G' || s=='T') fputc(s, fp2);
  }
	//printf("pos:%ld \r\n", ftell(fp));
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

int seeds(struct Options * op)
{
  FILE * database, * fp;
  struct CALs cals;
  long i, j;
  int seq;
  long pos;
  long dlen, index;
	char c;

	cals.addr = 1 << (op->m*2);
  cals.cal = (struct entry *)malloc(sizeof(struct entry) * cals.addr);

  for(i=0; i<cals.addr; i++)
  {
		cals.cal[i].len = 0;
		cals.cal[i].size = op->size != 0 ? op->size : 8;
		cals.cal[i].pos = (long *) malloc(sizeof(long)*cals.cal[i].size);
  }

  database = fopen(op->database,"r");
  if(database == NULL) { printf("Database cannot open:%s\r\n",op->database); return -1; }
	if(fgetc(database)!='e' || fgetc(database)!='x')
	{
		printf("In File Type Error\r\n");
		return -2;
	}

  seq = 0;
	fseek(database, 2, SEEK_SET);
	pos = 2;

	//printf("dlen:%ld \r\n", dlen);
	while(read_util(database, '\n', 0, NULL) == 0)
	{
		dlen = ftell(database) - pos - 2;
		//printf("dlen:%ld \r\n", dlen);
		for(i=0; i<dlen-op->m*op->n+op->m; i++)
 	  {
			index = 0;
 	    for(j=0; j<op->m; j++)
 	    {
 	      fseek(database, pos + i + j*op->n, SEEK_SET);
				c = fgetc(database);
				if(c=='A' || c=='C' || c=='G' || c=='T') index = (index << 2) + table[c];
				else
				{
					printf("Unexpected Char. seq:%d, dlen:%ld, pos:%ld\r\n", seq, dlen, pos);
					fclose(database);
					return -7;
				}
				//printf("c=%c, table[c]=%d, index=%ld\r\n", c, table[c], index);
 	    }

			if(index<0 || index >=cals.addr)
			{
				printf("Index error:%ld \r\n", index);
				fclose(database);
				return -4;
			}

			if(cals.cal[index].len < cals.cal[index].size)
			{
				cals.cal[index].pos[cals.cal[index].len++]=i;
				seq ++;
			}
			else
			{
				if(op->size == 0)
				{
					cals.cal[index].size += 8;
					cals.cal[index].pos = (long *)realloc(cals.cal[index].pos, cals.cal[index].size*sizeof(long));
					cals.cal[index].pos[cals.cal[index].len++]=i;
					seq ++;
				}
			}

		} 
		fseek(database, dlen + pos + 2, SEEK_SET);
		pos = dlen + pos + 2;
		//printf("len[%ld]=%d\r\n", index, cals.cal[index].len);
		//getchar();
	} 

  fp = fopen(op->filename,"wb");
  if(fp == NULL)
  {
    printf("Can create file:%s\r\n",op->filename);
    return -2;
  }

	fwrite(op, sizeof(struct Options), 1, fp);
	fwrite(&cals, sizeof(cals), 1, fp);
  for(i=0; i<cals.addr; i++)
  {
		fwrite(cals.cal+i, sizeof(struct entry), 1, fp);
		fwrite(cals.cal[i].pos, sizeof(long), cals.cal[i].len, fp);
		if(op->debug == 1) printf("len[%ld]=%d\r\n", i, cals.cal[i].len);
	}

  fclose(fp);
  fclose(database);
  fp = database = NULL;

  for(i=0; i<cals.addr; i++) free(cals.cal[i].pos);
	free(cals.cal);

  return seq;
}

int main(int argc, char ** argv)
{
  struct Options op;
  char c;
  int options;

  init_Options(&op);
  options = 0;
  while( (c=getopt(argc,argv,"m:n:s:d:f:D:")) >=0)
  {
    switch(c)
    {
      case 'm':options++; op.m = atoi(optarg); break;
      case 'n':options++; op.n = atoi(optarg); break;
      case 'd':options++; op.database = optarg; break;
      case 'f':options++; op.filename = optarg; break;
      case 's':op.size = atoi(optarg); break;
      case 'D':op.debug = 1; break;
      default: return print_help(argv[0]);
    }
  }
  if(options < 4) return print_help(argv[0]);
  format_Options(&op);
	if(op.debug == 1) dump_Options(&op);
  return seeds(&op);
}



