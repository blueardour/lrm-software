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
	fprintf(stdout, "  -l(ength) num *\r\n");
	fprintf(stdout, "  -i(nterval) num *\r\n");
	fprintf(stdout, "  -b(and) num *\r\n");
	fprintf(stdout, "  -r[eference] str *\r\n");
	fprintf(stdout, "  -p(refix) str\r\n");
	fprintf(stdout, "  -d(irectory) str\r\n");
	fprintf(stdout, "  -v(erbose)\r\n");
	fprintf(stdout, "  -V(ersion)\r\n");
	return 0;
}

static void dump_Options(struct Index_Options * op)
{
	fprintf(stdout, "%s ", PROGRAM);
	fprintf(stdout, "ver (%s):\r\n", VERSION);
	fprintf(stdout, "  -verbose 0x%02x\r\n", op->verbose);
	fprintf(stdout, "  -interval %d\r\n", op->interval);
	fprintf(stdout, "  -length %d\r\n", op->length);
	fprintf(stdout, "  -band %d\r\n", op->band);
	fprintf(stdout, "  -prefix %s\r\n", op->prefix);
	fprintf(stdout, "  -directory %s\r\n", op->dir);
	fprintf(stdout, "  -reference %s\r\n", op->database);
	fprintf(stdout, "  -pac %s\r\n", op->pac);
	fprintf(stdout, "  -uspt %s\r\n", op->uspt);
	fprintf(stdout, "  -si %s\r\n", op->si);
}

void init_Index_Options(struct Index_Options * op)
{
	op->verbose = -1;
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

static int format_Options(struct Index_Options * op)
{ 
	int len;
	char * path;

	len = 0;
	path = NULL;

	if(op->length < 1 || op->length > 65000) return -1;

	if(op->dir != NULL) path = op->dir; else path=getFilePath(op->database);
	if(path == NULL) path = "./";

	len = strlen(path) + strlen(getFileName(op->database));

	op->si = (char *)malloc(len + 6);
	op->pac = (char *)malloc(len + 6);
	op->uspt = (char *)malloc(len + 40);

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
	strcat(op->uspt, getFileName(op->database));

	if(op->prefix != NULL) strncat(op->uspt, op->prefix, 30);

	strcat(op->si, ".si");
	strcat(op->pac, ".pac");
	strcat(op->uspt, ".uspt");

	if(op->verbose & 0x80) op->verbose = 0x80;
	if(op->verbose & 0x40) op->verbose = 0x40;

	return 0;
}

static int generate_pac(struct Index_Options * op, struct Reference * ref)
{
	char * ptr, lc, c;
	FILE * database, * fp;
	int num, pnum, tmp;
	struct chromosome * chrptr;
	u32 len;

	fp = database = NULL;
	ref->A = ref->C = ref->G = ref->T = ref->N = 0;
	ref->seqs = 0;
	ref->chrom = NULL;

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

	fseek(database, 0, SEEK_SET);
	if(read2f_util(database, '>', 0, NULL, 0) < 0)
	{
		fprintf(stderr, "Error fa/fasta file format \r\n");
		if(fp != NULL) fclose(fp);
		fclose(database);
		return -3;
	}

	fp = fopen(op->pac, "w");
	if(fp == NULL)
	{
		fprintf(stderr, "Can create file:%s \r\n", op->pac);
		fclose(database); database = NULL;
		return -2;
	}

	num = 32;
	ref->chrom = (struct chromosome *) malloc(sizeof(struct chromosome)*num);

	while(op->verbose & 0x01)
	{
		if(feof(database) != 0) break;

		if(num == ref->seqs)
		{
			num += 8;
			ref->chrom = (struct chromosome *)realloc(ref->chrom, sizeof(struct chromosome)* num);
		}

		chrptr = ref->chrom + ref->seqs;
		chrptr->pnum = 0;
		pnum = 16;
		chrptr->slen = 0;
		chrptr->pie = (struct piece *) malloc(sizeof(struct piece)*pnum);

		ref->seqs++;

		chrptr->sn = (char *) malloc(1024);
		chrptr->nlen = 0;
		tmp = fscanf(database, "%s", chrptr->sn);
		if(tmp < 0 || tmp > 1024)
		{
			fprintf(stderr, "Failed to get ref name, err:%d\r\n", tmp);
			op->verbose = 0;
			if(fp != NULL) fclose(fp);
			if(database != NULL) fclose(database);
			return -2;
		}
		else
		{
			fprintf(stdout, "> %s...\r\n", chrptr->sn);
			chrptr->nlen = strlen(chrptr->sn);
		}

		if(read2f_util(database, '\n', 0, NULL, 0) < 0)
		{
			fprintf(stderr, "Failed to read ref comment\r\n");
			op->verbose = 0;
			if(fp != NULL) fclose(fp);
			if(database != NULL) fclose(database);
			return -2;
		}

		lc = 'N'; tmp = 0;
		chrptr->pie[chrptr->pnum].nb = 0;
		chrptr->pie[chrptr->pnum].plen = 0;
		while(c=fgetc(database), c != '>' && c != EOF)
		{
			if(isgraph(c) == 0)
			{
				continue;
			}
			else if(nst_nt4_table[(u08)c] < 4)
			{
				tmp = 0;
				lc = c;

				fputc(c, fp);
				chrptr->pie[chrptr->pnum].plen++;

				switch(nst_nt4_table[(u08)c])
				{
					case 0: ref->A++; break;
					case 1: ref->C++; break;
					case 2: ref->G++; break;
					case 3: ref->T++; break;
				}
			}
			else
			{
				if(lc == 'N') tmp++;
				lc = 'N';
				ref->N++;

				switch(newRand(1000000, 0) & 3)
				{
					case 0: c = 'A'; break;
					case 1: c = 'C'; break;
					case 2: c = 'G'; break;
					case 3: c = 'T'; break;
				}

				fputc(c, fp);

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
						chrptr->pie[chrptr->pnum].plen++;
					}
				}
			}
		}

		chrptr->pnum = chrptr->pnum + 1;

		for(tmp=0; tmp<chrptr->pnum; tmp++)
		{
			chrptr->slen += chrptr->pie[tmp].plen + chrptr->pie[tmp].nb;
		}

		fprintf(stdout, "\t%d\r\n", chrptr->slen);

		// finish all seqs or read unexpected char
		if(c == EOF)
		{
			// check file
			len = 0;
			for(tmp=0; tmp<ref->seqs; tmp++) len += chrptr->slen;
			if((ref->G + ref->A + ref->N + ref->T + ref->C) != len || len != ftell(fp))
			{
				fprintf(stderr, "> PAC file size not correct\r\n");
				if(database != NULL) fclose(database);
				if(fp != NULL) fclose(fp);
				return -3;
			}

			fclose(fp);
			fclose(database);
			fp = database = NULL;

			fp = fopen(op->si, "w");
			if(fp == NULL)
			{
				fprintf(stderr, "Can create file:%s \r\n", op->si);
				if(database != NULL) fclose(database);
				return -2;
			}

			fwrite(ref, sizeof(struct Reference), 1, fp);
			fwrite(ref->chrom, sizeof(struct chromosome), ref->seqs, fp);
			for(tmp=0; tmp<ref->seqs; tmp++)
			{
				chrptr = ref->chrom + tmp;
				fwrite(chrptr->pie, sizeof(struct piece), chrptr->pnum, fp);
				fwrite(chrptr->sn, sizeof(char), chrptr->nlen, fp);
				free(chrptr->pie);
				free(chrptr->sn);
			}

			fprintf(stdout, "> N:%d A:%d C:%d G:%d T:%d \r\n", ref->N, ref->A, ref->C, ref->G, ref->T);

			free(ref->chrom);
			fclose(fp);
			fp = NULL;
			break;
		}
	}
	return 0;
}

static int generate_uspt(struct Index_Options * op, struct Reference * ref)
{
	FILE * fp, * database;
	struct chromosome * chrptr;
	struct SPT_Header header;
	int tmp, pnum, num;
	char * buffer, * bbuffer;
	u32 i, cursor, position;
	Fingerprint pt, lpt, spt;

	fp = fopen(op->si, "r");
	if(fp == NULL)
	{
		fprintf(stderr, "File cannot open:%s\r\n", op->si);
		return -2;
	}

	if(fread(ref, sizeof(struct Reference), 1, fp) != 1)
	{
		fprintf(stderr, "SI File read error-1\r\n");
		return -2;
	}

	ref->chrom = (struct chromosome *) malloc(ref->seqs*sizeof(struct chromosome));
	if(fread(ref->chrom, sizeof(struct chromosome), ref->seqs, fp) != ref->seqs)
	{
		fprintf(stderr, "SI File read error-2\r\n");
		return -2;
	}

	for(tmp=0; tmp<ref->seqs; tmp++)
	{
		chrptr = ref->chrom + tmp;
		chrptr->pie = (struct piece *)malloc(sizeof(struct piece)*chrptr->pnum);
		chrptr->sn = (char *)malloc(chrptr->nlen + 1);
		chrptr->sn[chrptr->nlen] = 0;

		if(fread(chrptr->pie, sizeof(struct piece), chrptr->pnum, fp) != chrptr->pnum)
		{
			fprintf(stderr, "SI File read error-3\r\n");
			return -2;
		}
		if(fread(chrptr->sn, 1, chrptr->nlen, fp) != chrptr->nlen)
		{
			fprintf(stderr, "SI File read error-4\r\n");
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

	op->items = 0;
	lpt.pos = spt.pos = 0;
	for(tmp=0; tmp<FPSize; tmp++)
	{
		lpt.print[tmp] = 0;
		spt.print[tmp] = -1;
	}

	// fill SPT_Header
	header.items = op->items;
	header.length = op->length;
	header.interval = op->interval;
	header.band = op->band;
	header.size = FPSize;

	fwrite(&header, sizeof(struct SPT_Header), 1, fp);

	if(ftell(fp) != sizeof(struct SPT_Header))
	{
		fclose(database); fclose(fp);
		return -3;
	}

	cursor = 0;
	buffer = (char *) malloc(op->length + 1);
	buffer[op->length] = 0;
	bbuffer = (char *) malloc(op->length + 1);
	bbuffer[op->length] = 0;
	for(tmp=0; tmp<ref->seqs; tmp++)
	{
		chrptr = ref->chrom + tmp;

		for(pnum = 0, cursor += position = chrptr->pie[pnum].nb; \
				pnum < chrptr->pnum; \
				pnum++, \
				cursor += chrptr->pie[pnum-1].plen + chrptr->pie[pnum].nb,\
				position += chrptr->pie[pnum-1].plen + chrptr->pie[pnum].nb) 
		{
			if(chrptr->pie[pnum].plen < op->length) continue;

			for(i=0; i<=(chrptr->pie[pnum].plen-op->length); i+=op->interval)
			{
				fseek(database, cursor + i, SEEK_SET);
				if(fread(buffer, 1, op->length, database) != op->length)
				{
					fprintf(stderr, "Read File [%d in %d]. \r\n", cursor + i, chrptr->slen);
					fprintf(stderr, "Read File error[%d]. Err:%d, EOF:%d\r\n", \
							i, feof(database), ferror(database));
					return -1;
				}

				//if(buffer[op->length] == 0) printf("^1\r\n"); else printf("^0\r\n");
				strncpy(bbuffer, buffer + op->length/2, op->length - op->length/2 + 1);
				//printf("^2\r\n");
				strncat(bbuffer, buffer, op->length/2);
				//printf("^3\r\n");

				pt.pos = position + i + 1;
				stampFinger4(pt.print, buffer, op->length);
				//printf("^4\r\n");
				stampFinger4(pt.print + 4, bbuffer, op->length);
				//printf("^5\r\n");

				// debug
				//if(pt.pos > 79153100 && pt.pos < 79153200) printf("%d\r\n", pt.pos);
				//if(tmp==0 && pnum == 0 && i < 100) printf("%d\r\n", pt.pos);

				for(num=0; num<FPSize; num++)
				{
					lpt.print[num] = lpt.print[num] < pt.print[num] ? pt.print[num] : lpt.print[num];
					spt.print[num] = spt.print[num] > pt.print[num] ? pt.print[num] : spt.print[num];
				}

				//store fingerprint here
				fwrite(&pt, sizeof(pt), 1, fp);
				op->items++;
			} 
		} 
	}

	// re-write info
	cursor = ftell(fp);

	for(tmp=0; tmp<header.size; tmp++)
	{
		header.max[tmp] = lpt.print[tmp];
		header.min[tmp] = spt.print[tmp];
	}
	header.items = op->items;
	fseek(fp, 0, SEEK_SET);
	fwrite(&header, sizeof(struct SPT_Header), 1, fp);

	fseek(fp, 0, SEEK_END);
	if(cursor != ftell(fp))
	{
		fprintf(stderr, "Re-write uspt file error\r\n");
		return -1;
	}
	// end writting info

	fprintf(stdout, "> Built %d fingprinters\r\n", op->items);

	fclose(fp);
	fclose(database);
	free(buffer);
	for(tmp=0; tmp<ref->seqs; tmp++)
	{
		free(ref->chrom[tmp].sn);
		free(ref->chrom[tmp].pie);
	}
	free(ref->chrom);

	return 0;
}

int build_fingerprint(struct Index_Options * op)
{
	struct Reference ref;
	int tmp;

	tmp = format_Options(op);
	if(tmp < 0) return tmp;

	fprintf(stdout, "===> Dump parameters...%d\r\n", op->verbose & 0x80);
	if(op->verbose & 0x80)
	{
		dump_Options(op);
	}

	fprintf(stdout, "===> Generate PAC/SI file...%d\r\n", op->verbose & 0x01);
	if(op->verbose & 0x01)
	{
		tmp = generate_pac(op, &ref);
		if(tmp < 0) return tmp;
	}

	fprintf(stdout, "===> Generate uspt file...%d\r\n", op->verbose & 0x02);
	if(op->verbose & 0x02)
	{
		tmp = generate_uspt(op, &ref);
		if(tmp < 0) return tmp;
	}

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
	while( (c=getopt(argc,argv,"l:i:b:r:p:d:v:V")) >=0)
	{
		switch(c)
		{
			case 'l':options++; op.length = atoi(optarg); break;
			case 'i':options++; op.interval = atoi(optarg); break;
			case 'b':options++; op.band = atoi(optarg); break;
			case 'r':options++; op.database = optarg; break;
			case 'p': op.prefix = optarg; break;
			case 'd': op.dir = optarg; break;
			case 'v': op.verbose = atoi(optarg); break;
			case 'V': fprintf(stdout, "%s\r\n", VERSION); return 0;
			default: return print_help();
		}
	}

	if(options < 4) return print_help();

	return build_fingerprint(&op);
}

#endif


