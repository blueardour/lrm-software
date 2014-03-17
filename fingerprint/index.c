//  ******************************************************************//
//  author: chenp
//  description: build fingerprint database of the reference
//  version: 1.0
//	date: 2014-03-10
//  ******************************************************************//


#include "index.h"

#define VERSION "1.0"
#define PROGRAM "fblra index"

static int print_help()
{
	fprintf(stdout, "%s ", PROGRAM);
	fprintf(stdout, "ver (%s):\r\n", VERSION);
  fprintf(stdout, "  -v(erbose)\r\n");
  fprintf(stdout, "  -i(nterval) num\r\n");
  fprintf(stdout, "  -l(ength) num\r\n");
  fprintf(stdout, "  -b(and) num\r\n");
  fprintf(stdout, "  -r[eference] str\r\n");
  fprintf(stdout, "  -p(refix) str\r\n");
  fprintf(stdout, "  -d(irectory) str\r\n");
  fprintf(stdout, "  -V(ersion)\r\n");
  return 0;
}

void init_Index_Options(struct Index_Options * op)
{
	op->verbose = 3;
  op->length = 1000;
	op->interval = 128;
	op->band = 10;
	op->items = 0;
  op->database = NULL;
  op->prefix = NULL;
	op->pac = NULL;
	op->uspt = NULL;
	op->si = NULL;
	op->dir = NULL;
}

static void format_Options(struct Index_Options * op)
{ 
	int len;
	char * path;

	len = 0;
	path = NULL;

	if(op->dir != NULL) path = op->dir; else path=getFilePath(op->database);
	if(op->prefix != NULL) len += strlen(op->prefix);
	len = strlen(path) + strlen(getFileName(op->database));

	op->si = (char *)malloc(len+5);
	op->pac = (char *)malloc(len+5);
	op->uspt = (char *)malloc(len+5);

	strcpy(op->si, path);
	strcpy(op->pac, path);
	strcpy(op->uspt, path);

	if(path[strlen(path)-1] != '/')
	{
		op->si[strlen(path)] = '/';
		op->pac[strlen(path)] = '/';
		op->uspt[strlen(path)] = '/';
		op->si[strlen(path)+1] = 0;
		op->pac[strlen(path)+1] = 0;
		op->uspt[strlen(path)+1] = 0;
		len++;
	}

	strcat(op->si, getFileName(op->database));
	strcat(op->pac, getFileName(op->database));
	if(op->prefix != NULL) strcat(op->si, op->prefix);
	strcat(op->uspt, getFileName(op->database));

	len = len - strlen(getFileType(op->database));
	op->uspt[len] = 0;
	op->si[len] = 0;
	op->pac[len] = 0;
	strcat(op->si, "si");
	strcat(op->uspt, "uspt");
	strcat(op->pac, "pac");

	if(op->verbose & 0x80) op->verbose = 0x80;
}

static void dump_Options(struct Index_Options * op)
{
	fprintf(stdout, "%s ", PROGRAM);
	fprintf(stdout, "ver (%s):\r\n", VERSION);
  fprintf(stdout, "  -v(erbose) 0x%02x\r\n", op->verbose);
  fprintf(stdout, "  -i(nterval) %d\r\n", op->interval);
  fprintf(stdout, "  -l(ength) %d\r\n", op->length);
  fprintf(stdout, "  -b(and) %d\r\n", op->band);
  fprintf(stdout, "  -p(refix) %s\r\n", op->prefix);
  fprintf(stdout, "  -d(irectory) %s\r\n", op->dir);
  fprintf(stdout, "  -r[eference] %s\r\n", op->database);
  fprintf(stdout, "  -p[ac] %s\r\n", op->pac);
  fprintf(stdout, "  -u[spt] %s\r\n", op->uspt);
  fprintf(stdout, "  -s[i] %s\r\n", op->si);
}


int build_fingerprint(struct Index_Options * op)
{
	struct Reference ref;
	struct chromosome * chrptr;
	FILE * database, * fp;
	FType finger[8], print[8];
	FType max[8], min[8];
	u32 i, j;
	u32 cursor;
	char * buffer, * ptr;
	int tmp, num, pnum;
	char c, lc;
	
  if(op->length < 1 || op->length > 65000) return -1;

  format_Options(op);
	fp = database = NULL;
	buffer = ptr = NULL;
	cursor = 0;
	ref.A = ref.C = ref.G = ref.T = ref.N = 0;
	ref.seqs = 0;
	ref.chrom = NULL;

	ptr = getFileType(op->database);
	if(strcmp(ptr, "fa") != 0 && strcmp(ptr, "fasta") != 0)
	{
		fprintf(stderr, "Error: database type not support:%s\r\n", ptr);
		return -2;
	}

  database = fopen(op->database,"r");
  if(database == NULL)
  {
    fprintf(stderr, "Database cannot open:%s\r\n",op->database);
    return -2;
  }

	fprintf(stdout, "=> Parsing database:\r\n");
  fseek(database, 0, SEEK_SET);
	if(read2f_util(database, '>', 0, NULL, 0) < 0)
	{
		fprintf(stderr, "Error fa/fasta file format \r\n");
		fclose(fp);
		fclose(database);
		return -3;
	}

	fprintf(stdout, "===> Dump parameters...%d\r\n", op->verbose & 0x80);
	if(op->verbose & 0x80)
	{
		dump_Options(op);
	}

	fprintf(stdout, "===> Generate PAC/SI file...%d\r\n", op->verbose & 0x01);
	if(op->verbose & 0x01)
	{
		fp = fopen(op->pac, "w");
		if(fp == NULL)
		{
			fprintf(stderr, "Can create file:%s \r\n", op->pac);
			fclose(database); database = NULL;
			return -2;
		}
		
		num = 32;
		ref.chrom = (struct chromosome *) malloc(sizeof(struct chromosome)*num);
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
		chrptr->pnum = 0;
		pnum = 16;
		chrptr->pie = (struct piece *) malloc(sizeof(struct piece)*pnum);
		chrptr->slen = 0;
		chrptr->sn = (char *) malloc(50);
		chrptr->nlen = 0;
		ref.seqs++;

		tmp = fscanf(database, "%s", chrptr->sn);
		if(tmp < 0 || tmp > 50)
		{
			fprintf(stderr, "Failed to get ref name, err:%d\r\n", tmp);
			op->verbose = 0;
			break;
		}
		else
		{
			fprintf(stdout, "> %s...\r\n", chrptr->sn);
			chrptr->nlen = strlen(chrptr->sn);
		}

		if(read2f_util(database, '\n', 0, NULL, 0) < 0)
		{
			op->verbose = 0;
			break;
		}
		
		lc = 'N'; tmp = 0;
		chrptr->pie[chrptr->pnum].nb = 0;
		chrptr->pie[chrptr->pnum].plen = 0;
		while(c=fgetc(database), c != '>' && c != EOF)
		{
			if(c == 'N')
			{
				if(lc == 'N') tmp++;
				lc = c;
				ref.N++;

				if(chrptr->pie[chrptr->pnum].plen == 0)
				{
					chrptr->pie[chrptr->pnum].nb++;
				}
				else
				{
					if(tmp >= op->band)
					{
						if(chrptr->pnum == (pnum-1))
						{
							pnum += 8;
							chrptr->pie = (struct piece *) realloc(chrptr->pie, sizeof(struct piece)*pnum);
						}
						tmp = 1;
						chrptr->pnum++;
						chrptr->pie[chrptr->pnum].nb = 1;
						chrptr->pie[chrptr->pnum].plen = 0;
					}
					else
					{
						fputc(c, fp);
						chrptr->pie[chrptr->pnum].plen++;
					}
				}
			}
			else if(c=='A' || c=='C'|| c=='G' || c=='T')
			{
				tmp = 0;
				lc = c;

				fputc(c, fp);
				chrptr->pie[chrptr->pnum].plen++;

				if(c=='A') ref.A++;
				if(c=='C') ref.C++;
				if(c=='T') ref.T++;
				if(c=='G') ref.G++;
			}
			else if(c=='\r' || c== '\n')
			{
				continue;
			}
			else
			{
				fprintf(stderr, "Unexpected char(%d) when read seq[%d] \r\n", c, ref.seqs);
				op->verbose = 0;
				break;
			}
		}

		chrptr->pnum++;

		for(tmp=0; tmp<chrptr->pnum; tmp++)
		{
			chrptr->slen += chrptr->pie[tmp].plen + chrptr->pie[tmp].nb;
		}
		
		fprintf(stdout, "\t%d\r\n", chrptr->slen);

		if(c != '>') // finish all seqs or read unexpected char
		{
			fclose(fp);
			fclose(database);
			fp = database = NULL;

			fp = fopen(op->si, "w");
			if(fp == NULL)
			{
				fprintf(stderr, "Can create file:%s \r\n", op->si);
				fclose(database);
				return -2;
			}

			fwrite(&ref, sizeof(struct Reference), 1, fp);
			fwrite(ref.chrom, sizeof(struct chromosome), ref.seqs, fp);
			for(tmp=0; tmp<ref.seqs; tmp++)
			{
				chrptr = ref.chrom + tmp;
				fwrite(chrptr->pie, sizeof(struct piece), chrptr->pnum, fp);
				fwrite(chrptr->sn, sizeof(char), chrptr->nlen, fp);
				free(chrptr->pie);
				free(chrptr->sn);
			}

			free(ref.chrom);
			fclose(fp);
			fp = NULL;

			fprintf(stdout, "> N:%d A:%d C:%d G:%d T:%d \r\n", ref.N, ref.A, ref.C, ref.G, ref.T);
			break;
		}
	}

	fprintf(stdout, "===> Generate uspt file...%d\r\n", op->verbose & 0x02);
	if(op->verbose & 0x02)
	{
		fp = fopen(op->si, "r");
		if(fp == NULL)
		{
			fprintf(stderr, "File cannot open:%s\r\n", op->si);
			return -2;
		}

		if(fread(&ref, sizeof(struct Reference), 1, fp) != 1)
		{
			fprintf(stderr, "SI File read error\r\n");
			return -2;
		}

		ref.chrom = (struct chromosome *) malloc(ref.seqs*sizeof(struct chromosome));
		if(fread(ref.chrom, sizeof(struct chromosome), ref.seqs, fp) != ref.seqs)
		{
			fprintf(stderr, "SI File read error\r\n");
			return -2;
		}

		for(tmp=0; tmp<ref.seqs; tmp++)
		{
			chrptr = ref.chrom + tmp;
			chrptr->pie = (struct piece *)malloc(sizeof(struct piece)*chrptr->pnum);
			chrptr->sn = (char *)malloc(chrptr->nlen + 1);
			chrptr->sn[chrptr->nlen] = 0;

			if(fread(chrptr->pie, sizeof(struct piece), chrptr->pnum, fp) != chrptr->pnum)
			{
				fprintf(stderr, "SI File read error-3(EOF:%d, ERR:%d)\r\n", feof(fp), ferror(fp));
				return -2;
			}
			if(fread(chrptr->sn, 1, chrptr->nlen, fp) != chrptr->nlen)
			{
				fprintf(stderr, "SI File read error-4(EOF:%d, ERR:%d)\r\n", feof(fp), ferror(fp));
				return -2;
			}
		}

		fclose(fp);

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
		cursor = 0;
		op->items = 0;
		max[0]=max[1]=max[2]=max[3]=max[4]=max[5]=max[6]=max[7]=0;
		min[0]=min[1]=min[2]=min[3]=min[4]=min[5]=min[6]=min[7]=-1;

		fwrite(&op->items, sizeof(op->items), 1, fp);
		fwrite(max, sizeof(FType), 8, fp);
		fwrite(min, sizeof(FType), 8, fp);
		fwrite(&op->length, sizeof(op->length), 1, fp);
		fwrite(&op->interval, sizeof(op->interval), 1, fp);
		fwrite(&op->band, sizeof(op->band), 1, fp);

		for(tmp=0; tmp<ref.seqs; tmp++)
		{
			chrptr = ref.chrom + tmp;
			for(pnum=0; pnum<chrptr->pnum; pnum++)
			{
				if(chrptr->pie[pnum].plen < op->length)
				{
					cursor += chrptr->pie[pnum].plen;;
					continue;
				}

				for(i=0; i<=(chrptr->pie[pnum].plen-op->length); i+=op->interval)
				{
					// cursor represents the beginning postion of current piece
					fseek(database, cursor + i, SEEK_SET);
					if(fread(buffer, 1, op->length, database) != op->length)
					{
						fprintf(stderr, "Read File error. Err:%d-EOF:%d\r\n", feof(database), ferror(database));
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
					
					for(j=0; j<8; j++)
					{
						if(max[j] < print[j]) max[j] = print[j];
						if(min[j] > print[j]) min[j] = print[j];
					}
	
					//store fingerprint here
					j = cursor + i;
					fwrite(&j, sizeof(j), 1, fp);
					fwrite(print, sizeof(FType), 8, fp);
					op->items++;

				} // for(i=0; i<X; i+=op->interval)

				cursor += chrptr->pie[pnum].plen;
			} // for(pnum=0; pnum < chrptr->pnum; pnum++)
		} // for(tmp=0; tmp<ref.seqs; tmp++)

		// re-write info
		cursor = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		fwrite(&op->items, sizeof(op->items), 1, fp);
		fwrite(max, sizeof(FType), 8, fp);
		fwrite(min, sizeof(FType), 8, fp);
		fseek(fp, 0, SEEK_END);
		if(cursor != ftell(fp))
			fprintf(stderr, "Re-write uspt file error\r\n");
		
		fclose(fp);
		fclose(database);

		fprintf(stdout, "> Built %d fingprinters\r\n", op->items);

		free(buffer);
		for(tmp=0; tmp<ref.seqs; tmp++)
		{
			free(ref.chrom[tmp].sn);
			free(ref.chrom[tmp].pie);
		}
		free(ref.chrom);
	} // if

  return 0;
}


#ifndef _FINGERPRINT_MAIN_C_

int main(int argc, char ** argv)
{
  struct Index_Options op;
  char c;
  int options;

  init_Index_Options(&op);
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


