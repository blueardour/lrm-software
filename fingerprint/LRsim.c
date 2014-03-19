//  ******************************************************************//
//  author: chenp
//  version: 4.1
//  description: generate certain amount of reads as the simulated data
//  date: 2012-6-27
//	update: 2014-02-24 with postion information
//	update: 2014-02-25, change gaps insertion strategyy 
//	update: 2014-03-06, change query name format
//  ******************************************************************//


#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "utils.h"
#include "fingerprint.h"

#define VERSION "1.0"
#define PROGRAM "LRsim"

struct LRS_Options
{
  u08 error;				// percent
  u32 length;
  u32 amount;
	float vr;
  char * database;  // filename
  char * filename;
	char * si;
	char * pac;
};

struct Indels
{
	u08 type;
	u32 pos;
	u32 len;
};

int print_help()
{
  printf("%s ", PROGRAM);
  printf("ver (%s):\r\n", VERSION);
  printf("	-V(ersion)\r\n");
  printf("	-a(mount)\r\n");
  printf("	-l(ength) num\r\n");
  printf("	-e(rror) num\r\n");
  printf("	-d(atabase)\r\n");
  printf("	-f(ilename) str\r\n");
  return 0;
}

void init_LRS_Options(struct LRS_Options * op)
{
  op->length = 1000;
  op->error = 3;
	op->vr = 0.8;
  op->amount = 10000;
  op->database = NULL;
  op->filename = NULL;
	op->si = NULL;
	op->pac = NULL;
}

void format_Options(struct LRS_Options * op)
{ 
	int len;
	len = strlen(op->database);
	op->si = (char *)malloc(len+6);
	op->pac = (char *)malloc(len+6);

	strcpy(op->si, op->database);
	strcpy(op->pac, op->database);
	strcat(op->si, ".si");
	strcat(op->pac, ".pac");
}

void dump_Options(struct LRS_Options * op)
{ 
}

int generate_queries(struct LRS_Options * op)
{
	struct Reference ref;
	struct chromosome * chrptr;
	struct Indels * gaps;
  FILE * database, * fp;
	int tmp, loop, chr, pie;
	time_t tv;
  u32 i, j, k, l;
  u32 position, alen, cursor;
  char * buffer, * string;
  char c;

  format_Options(op);

  fp = fopen(op->si, "r");
  if(fp == NULL)
  {
    printf("Database cannot open:%s\r\n", op->si);
    return -2;
  }

	if(fread(&ref, sizeof(struct Reference), 1, fp) != 1) return -3;
	ref.chrom = (struct chromosome *) malloc(sizeof(struct chromosome)*ref.seqs);
	if(fread(ref.chrom, sizeof(struct chromosome), ref.seqs, fp) != ref.seqs) return -3;
	for(tmp=0; tmp<ref.seqs; tmp++)
	{
		chrptr = ref.chrom + tmp;
		chrptr->pie = (struct piece *) malloc(sizeof(struct chromosome)*chrptr->pnum);
		chrptr->sn = (char *) malloc(sizeof(char)*chrptr->nlen);
		if(fread(chrptr->pie, sizeof(struct piece), chrptr->pnum, fp) != chrptr->pnum) return -3;
		if(fread(chrptr->sn, sizeof(char), chrptr->nlen, fp) != chrptr->nlen) return -3;
	}
	fclose(fp);

  database = fopen(op->pac, "r");
  if(database == NULL)
  {
    printf("Database cannot open:%s\r\n", op->pac);
    return -2;
  }

  fp = fopen(op->filename, "w");
  if(fp == NULL)
  {
    printf("Can create file:%s\r\n", op->filename);
    return -2;
  }

	alen = op->length * op->error / 100;
  buffer = (char *)malloc(op->length);
  string = (char *)malloc(op->length + alen);
	gaps = (struct Indels *) malloc(sizeof(struct Indels) * (u32)(alen*(1-op->vr)));
	c = 'A';

  for(i=0; i<op->amount; i++)
  {
		chr = newRand(ref.seqs);
		chrptr = ref.chrom + chr;

		pie = newRand(chrptr->pnum);
		if(chrptr->pie[pie].plen <= op->length)
		{
			i++;
			continue;
		}

		position = newRand(chrptr->pie[pie].plen - op->length);

		cursor = 0;
		for(tmp=0; tmp<chr; tmp++)
		{
			chrptr = ref.chrom + tmp;
			for(loop=0; loop<chrptr->pnum; loop++)
			{
				cursor += chrptr->pie[loop].plen;
			}
		}

		chrptr = ref.chrom + chr;
		for(loop=0; loop<pie; loop++)
			cursor += chrptr->pie[loop].plen;

		if(fseek(database, cursor + position, SEEK_SET) != 0) return -4;

		for(j=0; j<op->length; j++)
		{
			c= getc(database);
			if(c=='A' || c== 'C' || c=='G' || c== 'T') buffer[j] = c;
			else
			{
				printf("Unexpected char:%d \r\n", c);
				return -2;
			}
		}

		for(j=0; j<alen; j++)
		{
			if(j < (u32)(alen*op->vr))
			{
				switch(newRand(4))
				{
					case 0: c= 'A'; break;
					case 1: c= 'C'; break;
					case 2: c= 'G'; break;
					case 3: c= 'T'; break;
				}
				buffer[newRand(op->length)] = c;
			}
			else
			{
				gaps[j].pos = newRand(op->length);
				gaps[j].len = 1;
				gaps[j].type = 0; // addition:non-zero, deletion:zero
				do {
					tmp = newRand(20);
					if(tmp<=6 && tmp>=0)
					{
						break;
					}
					else if(tmp>=7 && tmp<=13)
					{
						gaps[j].type = ! gaps[j].type;
						break;
					}
					gaps[j].len++;
					if(gaps[j].len > (u32)alen*(1-op->vr))
					{
						gaps[j].type = 0;
						break;
					}
					else gaps[j].type = ! gaps[j].type;
				} while(1);
			}
		}

		// final string 
		for(j=l=0; j<op->length; j=j)
		{
			for(k=0; k<alen/5; k++)
			{
				if(gaps[k].pos == j)
				{
					if(gaps[k].type == 0)
						j+=gaps[k].len;
					else
					{
						string[l++] = buffer[j++];
						while(gaps[k].len--) string[l++] = buffer[op->length-1-gaps[k].len];
					}
					break;
				}
			}

			if(k == alen/5) string[l++] = buffer[j++];
		}

		fprintf(fp, ">%s-%d-%d %d-%d-%d-len:%d\r\n%s\r\n", getFileName(op->database), i, cursor + position, \
								chr, pie, position, l, string);
		
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
  struct LRS_Options op;
  char c;
  int options;

  init_LRS_Options(&op);
  options = 0;
  while( (c=getopt(argc,argv,"l:a:d:e:f:V")) >=0)
  {
    switch(c)
    {
      case 'l':options++; op.length = atoi(optarg); break;
      case 'a':options++; op.amount = atoi(optarg); break;
      case 'e':options++; op.error  = atoi(optarg); break;
      case 'f':options++; op.filename = optarg; break;
      case 'd':options++; op.database = optarg; break;
			case 'V':printf("%s\r\n", VERSION); return 0;
      default: return print_help();
    }
  }
  if(options < 5) return print_help();
  return generate_queries(&op);
}



