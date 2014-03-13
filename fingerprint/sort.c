//  ******************************************************************//
//  author: chenp
//  description: sort fingerprint database of the reference
//  version: 1.0
//	date: 2014-03-12
//  ******************************************************************//


#include "sort.h"

#define VERSION "1.0"
#define PROGRAM "fblra sort"


struct Sort_Options
{
};


static int print_help()
{
	fprintf(stdout, "%s ", PROGRAM);
	fprintf(stdout, "ver (%s):\r\n", VERSION);
  fprintf(stdout, "  -v(erbose)\r\n");
  fprintf(stdout, "  -u[spt] str\r\n");
  fprintf(stdout, "  -p(attern) str\r\n");
  fprintf(stdout, "  -V(ersion)\r\n");
  return 0;
}

static void init_Options(struct Sort_Options * op)
{
	op->verbose = 3;
  op->prefix = NULL;
	op->pattern = NULL;
	op->si = NULL;
	op->uspt = NULL;
	op->spt = NULL;
}

static void format_Options(struct Sort_Options * op)
{ 
	int len;

	if(op->prefix != NULL) len = strlen(op->prefix);

	op->si = (char *)malloc(len+6);
	op->spt = (char *)malloc(len+6);
	op->uspt = (char *)malloc(len+6);

	strcpy(op->si, op->prefix);
	strcpy(op->spt, op->prefix);
	strcpy(op->uspt, op->prefix);

	strcat(op->si, ".si");
	strcat(op->uspt, ".uspt");
	strcat(op->spt, ".spt");

	if(op->verbose & 0x80) op->verbose = 0x80;
}

static void dump_Options(struct Sort_Options * op)
{
	fprintf(stdout, "%s ", PROGRAM);
	fprintf(stdout, "ver (%s):\r\n", VERSION);
  fprintf(stdout, "  -v(erbose) 0x%02x\r\n", op->verbose);
  fprintf(stdout, "  -p[attern] %s\r\n", op->pattern);
  fprintf(stdout, "  -u(spt) %s\r\n", op->uspt);
  fprintf(stdout, "  -s[pt] %s\r\n", op->spt);
  fprintf(stdout, "  -s[i] %s\r\n", op->si);
}


int sort_fingerprint(struct Sort_Options * op)
{
	struct Reference ref;
	struct chromosome * chrptr;
	FILE * database, * fp;
	FType finger[8], print[8];
	u32 i, j;
	u32 cursor;
	char * buffer, * ptr;
	int tmp, num;
	char c;
	
  format_Options(op);
	fp = database = NULL;
	buffer = ptr = NULL;

	ptr = getFileType(op->uspt);
	if(strcmp(ptr, "uspt") != 0)
	{
		fprintf(stderr, "Error: database type not support:%s\r\n", ptr);
		return -2;
	}

  database = fopen(op->uspt, "rb");
  if(database == NULL)
  {
    fprintf(stderr, "Database cannot open:%s\r\n", op->uspt);
    return -2;
  }

	tmp = fread(&ref, sizeof(ref), 1, database);
	if(tmp < 0)
	{
		fprintf(stderr, "Read database error-1.\r\n");
		return -2;
	}

	ref.chrom = (struct chromosome *) malloc(sizeof(struct chromosome)*ref.seqs);
	tmp = fread(ref.chrom, sizeof(ref.chrom), ref.seqs, database);
	if(tmp < 0)
	{
		fprintf(stderr, "Read database error-2.\r\n");
		return -2;
	}

	fprintf(stdout, "===> Dump parameters...%d\r\n", op->verbose & 0x80);
	if(op->verbose & 0x80)
	{
		dump_Options(op);
	}

	fprintf(stdout, "===> Generate PAC file...%d\r\n", op->verbose & 0x01);
	if(op->verbose & 0x01)
	{
		fp = fopen(op->pac, "w");
		if(fp == NULL)
		{
			fprintf(stderr, "Can create file:%s \r\n", op->pac);
			fclose(database);
			return -2;
		}
	}

	while(op->verbose & 0x01)
	{
		if(feof(database) != 0) break;

		if(num == ref.seqs)
		{
			num += 8;
			ref.chrom = (struct chromosome *)realloc(ref.chrom, sizeof(struct chromosome)* num);
		}

		chrptr = ref.chrom + ref.seqs;
		chrptr->nlen = 0;
		chrptr->nb = chrptr->ne = 0;
		chrptr->slen = 0;

		chrptr->sn = (char *) malloc(50);
		tmp = fscanf(database, "%s", chrptr->sn);
		if(tmp < 0 || tmp > 50)
		{
			fprintf(stderr, "Failed to get ref name, err:%d\r\n", tmp);
			break;
		}
		else
		{
			fprintf(stdout, "> %s...\r\n", chrptr->sn);
			chrptr->nlen = strlen(chrptr->sn);
		}

		if(read2f_util(database, '\n', 0, NULL, 0) < 0) break;

		do {
			c = fgetc(database);
			if(c == 'N') chrptr->nb++;
			else if(c=='\r' || c=='\n') continue;
			else break;
		} while(1);

		if(c==EOF) break;
		ref.seqs++;

		fseek(database, -1, SEEK_CUR);
		tmp = read2f_util(database, '>', 0x02| 0x01, fp, op->band);

		if(tmp == -3)		// read band 'N' before '>'
		{
			chrptr->slen = ftell(fp) - op->band;
			fseek(database, chrptr->slen, SEEK_SET);

			chrptr->ne = op->band;
			do { 
				c = fgetc(database);
				if(c=='N') chrptr->ne++;
			} while(c != EOF && c != '>');

		}
		else
		{
			chrptr->slen = ftell(fp);
			chrptr->ne = 0;
		}
	}

	if(fp != NULL) fclose(fp);
	if(database != NULL) fclose(database);
	fp = database = NULL;

	fprintf(stdout, "===> Generate si file...%d\r\n", op->verbose & 0x01);
	if(op->verbose & 0x01)
	{
	  database = fopen(op->pac, "r");
		if(database == NULL)
	  {
		  fprintf(stderr, "Database cannot open:%s\r\n",op->pac);
			return -2;
	  }

		fp = fopen(op->si, "wb");
		if(fp == NULL)
		{
			fprintf(stderr, "Can create file:%s \r\n", op->si);
			fclose(database);
			return -2;
		}

		while((c=fgetc(database)) != EOF)
		{
			if(c=='A') ref.A++;
			else if(c=='C') ref.C++;
			else if(c=='G') ref.G++;
			else if(c=='T') ref.T++;
			else if(c=='N') ref.N++;
			else
			{
				fprintf(stderr, "Fatal error: unexpectd char(%d)\r\n", c);
				break;
			}
		}

		fwrite(&ref, sizeof(ref), 1, fp);
		fwrite(ref.chrom, sizeof(struct chromosome), ref.seqs, fp);
		for(tmp=0; tmp<ref.seqs; tmp++)
		{
			fwrite(ref.chrom[tmp].sn, 1, ref.chrom[tmp].nlen, fp);
			free(ref.chrom[tmp].sn);
		}

		fclose(database);
		fclose(fp);
		fp = database = NULL;
	}

	fprintf(stdout, "===> Generate uspt file...%d\r\n", op->verbose & 0x02);
	if(op->verbose & 0x02)
	{
		if((op->verbose & 0x01) == 0x00)
		{
			fp = fopen(op->si, "rb");
			if(fp == NULL)
			{
				fprintf(stderr, "File cannot open:%s\r\n", op->si);
				return -2;
			}

			tmp = fread(&ref, sizeof(ref), 1, fp);
			if(tmp < 0)
			{
				fprintf(stderr, "File read error from SI file-1\r\n");
				return -2;
			}

			ref.chrom = (struct chromosome *) malloc(ref.seqs*sizeof(struct chromosome));
			tmp = fread(ref.chrom, sizeof(struct chromosome), ref.seqs, fp);
			if(tmp < 0)
			{
				fprintf(stderr, "File read error from SI file-2\r\n");
				return -2;
			}
		}

	  database = fopen(op->pac, "r");
		if(database == NULL)
	  {
		  fprintf(stderr, "Database cannot open:%s\r\n", op->pac);
			return -2;
	  }

		fp = fopen(op->uspt, "wb");
		if(fp == NULL)
		{
			fprintf(stderr, "Can create file:%s \r\n", op->uspt);
			fclose(database);
			return -2;
		}

		buffer = (char *) malloc(op->length);

		fwrite(&op->items, sizeof(op->items), 1, fp);
		fwrite(&op->length, sizeof(op->length), 1, fp);
		fwrite(&op->interval, sizeof(op->interval), 1, fp);
		fwrite(&op->band, sizeof(op->band), 1, fp);

		cursor = 0;
		op->items = 0;

		for(tmp=0; tmp<ref.seqs; tmp++)
		{
			chrptr = ref.chrom + tmp;
			for(i=0; i<(chrptr->slen-op->length); i+=op->interval)
			{
				fseek(database, cursor + i, SEEK_SET);
				if(fread(buffer, 1, op->length, database) != op->length)
				{
					fprintf(stderr, "Read File read. Err:%d-EOF:%d\r\n", feof(database), ferror(database));
					break;
				}

				finger[0]=finger[1]=finger[2]=finger[3]=0;
				finger[4]=finger[5]=finger[6]=finger[7]=op->length;
				memset(print, 0, sizeof(FType)*8);
				for(j=0; j<op->length; j++)
				{
					print[4] += finger[4];
					print[5] += finger[5];
					print[6] += finger[6];
					print[7] += finger[7];

					switch(buffer[j])
					{
						case 'A': finger[0]++; finger[4]--; break;
						case 'C': finger[1]++; finger[5]--; break;
						case 'G': finger[2]++; finger[6]--; break;
						case 'T': finger[3]++; finger[7]--; break;
					}

					print[0] += finger[0];
					print[1] += finger[1];
					print[2] += finger[2];
					print[3] += finger[3];
				}

				print[4] -= finger[4]*op->length;
				print[5] -= finger[5]*op->length;
				print[6] -= finger[6]*op->length;
				print[7] -= finger[7]*op->length;
	
				//store fingerprint here
				j = chrptr->nb + i;
				fwrite(&tmp, sizeof(tmp), 1, fp);
				fwrite(&j, sizeof(j), 1, fp);
				fwrite(print, sizeof(FType), 8, fp);
				op->items++;
			
			} // for(i=0; i<(chrptr->slen-op->length); i+=op->interval)
			cursor += chrptr->slen;
		} // for(tmp=0; tmp<ref.seqs; tmp++)

		fseek(database, 0, SEEK_SET);
		fwrite(&op->items, sizeof(op->items), 1, fp);
	} // if


	if(ref.chrom != NULL) free(ref.chrom);
  if(buffer != NULL) free(buffer);
  if(fp != NULL) fclose(fp);
  if(database != NULL) fclose(database);
  return 0;
}


#ifndef _FINGERPRINT_MAIN_C_

int main(int argc, char ** argv)
{
  struct Index_Options op;
  char c;
  int options;

  init_Options(&op);
  options = 0;
  while( (c=getopt(argc,argv,"i:l:b:r:p:d:v:V")) >=0)
  {
    switch(c)
    {
      case 'i':options++; op.interval = atoi(optarg); break;
      case 'l':options++; op.length = atoi(optarg); break;
      case 'b':options++; op.band = atoi(optarg); break;
      case 'r':options++; op.database = optarg; break;
      case 'p': op.prefix = optarg; break;
      case 'd': op.dir = optarg; break;
      case 'v': op.verbose = atoi(optarg); break;
      case 'V': fprintf(stdout, "%s\r\n", VERSION); return 0;
      default: return print_help(getFileName(argv[0]));
    }
  }

  if(options < 4) return print_help(getFileName(argv[0]));

  return build_fingerprint(&op);
}

#endif

