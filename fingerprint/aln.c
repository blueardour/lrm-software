//  ******************************************************************//
//  author: chenp
//  description: aligner 
//  version: 1.0
//	date: 2014-03-20
//  ******************************************************************//


#include "aln.h"

#define VERSION "1.0"
#define PROGRAM "fblra aln"

static int print_help()
{
	fprintf(stdout, "%s ", PROGRAM);
	fprintf(stdout, "ver (%s):\r\n", VERSION);
  fprintf(stdout, "  -v(erbose)\r\n");
  fprintf(stdout, "  -d(irectory) str\r\n");
  fprintf(stdout, "  -V(ersion)\r\n");
  fprintf(stdout, "  -spt str r\n");
  fprintf(stdout, "  -read str\r\n");
  fprintf(stdout, "  -sam str\r\n");
  return 0;
}

void init_ALN_Options(struct ALN_Options * op)
{
	op->verbose = 3;
  op->length = 1000;
	op->interval = 128;
	op->band = 10;
	op->items = 0;
	op->pt = NULL;
	op->index = NULL;
	op->pac = NULL;
	op->spt = NULL;
	op->read = NULL;
	op->sam = NULL;
}

static void format_Options(struct ALN_Options * op)
{ 
	op->pac = (char *)malloc(strlen(op->index) + 5);
	strcpy(op->pac, op->index);
	strcat(op->pac, ".pac");

	if(op->verbose & 0x80) op->verbose = 0x80;
}

static void dump_Options(struct ALN_Options * op)
{
	fprintf(stdout, "%s ", PROGRAM);
	fprintf(stdout, "ver (%s):\r\n", VERSION);
  fprintf(stdout, "  -v(erbose) 0x%02x\r\n", op->verbose);
  fprintf(stdout, "  -i(nterval) %d\r\n", op->interval);
  fprintf(stdout, "  -l(ength) %d\r\n", op->length);
  fprintf(stdout, "  -b(and) %d\r\n", op->band);
  fprintf(stdout, "  -r[eference] %s\r\n", op->index);
  fprintf(stdout, "  -spt %s\r\n", op->spt);
  fprintf(stdout, "  -read %s\r\n", op->read);
  fprintf(stdout, "  -sam %s\r\n", op->sam);
}

static int load_spt(struct ALN_Options * op)
{
	struct Fingerprint pt;
	FILE * fp;
	int offset, tmp;
	u32 i;

	offset = sizeof(FType)*16 + sizeof(u32)*4 + 20;

	fp = fopen(op->spt, "rb");
	if(fp == NULL) 
	{
		fprintf(stderr, "Database cannot open:%s\r\n", op->spt);
		return -2;
	}

	if(fread(&op->items, sizeof(op->items), 1, fp) != 1) return -3;
	fseek(fp, sizeof(u32) + 16 * sizeof(FType), SEEK_SET);
	if(fread(&op->length, sizeof(op->length), 1, fp) != 1) return -3;
	if(fread(&op->interval, sizeof(op->interval), 1, fp) != 1) return -3;
	if(fread(&op->band, sizeof(op->band), 1, fp) != 1) return -3;
	if(fread(op->pattern, 1, 4, fp) != 1) return -3;

	fprintf(stdout, "> Going to read %d fingerprint. len:%d interval:%d band:%d\r\n", \
					op->items, op->length, op->interval, op->band);
	op->pt = (struct PT *) malloc(sizeof(struct PT) * op->items);

	fseek(fp, offset, SEEK_SET);
	for(i=0; i<op->items; i++)
	{
		if(fread(&pt, sizeof(struct Fingerprint), 1, fp) != 1) return -3;
		op->pt[i].pos = pt.pos;
		for(tmp=0; tmp<4; tmp++) op->pt[i].print[tmp] = pt.print[tmp];
	}
	fclose(fp);
	return 0;
}

static inline FType value(FType a, FType b)
{
	if(a > b) return a - b; else return b - a;
}

static FType estimate(FType * pt1, FType * pt2)
{
	return value(pt1[0], pt2[0]) + value(pt1[1], pt2[1]) + value(pt1[2], pt2[2]) + value(pt1[3], pt2[3]);
}

static u32 search_similar(struct PT * list, u32 len, FType pt)
{
	u32 beg, end, i;
	struct PT * ptr;

	beg = 0;
	end= len;
	while(1)
	{
		i = end - beg;
		if(i < 20) return beg + i/2;
		ptr = list + beg;
		if(ptr[i/2].print[0] < pt) end -= i/2; else beg += i/2;
	}
	return 0;
}

static int align_read(struct ALN_Options * op)
{
	FILE * pac, * read, * sam;
	char sn[100];
	char * buffer;
	int tmp;
	u32 i, begin, end;
	FType finger[8], print[8];
	struct Differ MinDiff;

	pac = fopen(op->pac, "r");
	if(pac == NULL) 
	{
		fprintf(stderr, "Database cannot open:%s\r\n", op->pac);
		return -2;
	}

	read = fopen(op->read, "r");
	if(read == NULL) 
	{
		fprintf(stderr, "Database cannot open:%s\r\n", op->read);
		return -2;
	}

	sam = fopen(op->sam, "r");
	if(read == NULL) 
	{
		fprintf(stderr, "Cannot create file:%s\r\n", op->sam);
		return -2;
	}


	buffer = (char *)malloc(op->length + 1);

	while(1)
	{
		if(feof(read) != 0) break;

		if(read2b_util(read, '>', 0, NULL, 0) < 0) return -4;

		tmp = fscanf(read, "%s", sn);
		if(read2b_util(read, '\n', 0, NULL, 0) < 0) break;

		if(read2b_util(read, '>', 1, buffer, op->length) != 0) break;

		finger[0] = finger[1] = finger[2] = finger[3] = 0;
		print[0] = print[1] = print[2] = print[3] = 0;
		for(i=0; i<op->length; i++)
		{
			if(buffer[i] == op->pattern[0]) finger[0]++;
			else if(buffer[i] == op->pattern[1]) finger[1]++;
			else if(buffer[i] == op->pattern[2]) finger[2]++;
			else finger[3]++;

			print[0] += finger[0];
			print[1] += finger[1];
			print[2] += finger[2];
			print[3] += finger[3];
		}

		i = search_similar(op->pt, op->items, print[0]);
		begin = i > op->threshold ? i - op->threshold : 0;
		end = i + op->threshold > op->items? op->items : i + op->threshold;

		MinDiff.diff = estimate(print, op->pt[end].print);
		MinDiff.pos = end;
		for(i=begin; i<end; i++)
		{
			if(MinDiff.diff < estimate(print, op->pt[i].print))
			{
				MinDiff.diff = estimate(print, op->pt[i].print);
				MinDiff.pos =  op->pt[i].pos;
			}
		}
		fprintf(stdout, "%s\t%d\r\n", sn, MinDiff.pos);
	}

	return 0;
}

int aln_by_fingerprint(struct ALN_Options * op)
{
	int tmp;

  format_Options(op);

	fprintf(stdout, "===> Dump parameters...%d\r\n", op->verbose & 0x80);
	if(op->verbose & 0x80)
	{
		dump_Options(op);
	}

	fprintf(stdout, "===> Load database...%d\r\n", op->verbose & 0x01);
	if(op->verbose & 0x01)
	{
		tmp = load_spt(op);
		if(tmp < 0) return tmp;
	}

	fprintf(stdout, "===> Align reads...%d\r\n", op->verbose & 0x03);
	if((op->verbose & 0x03) == 0x03)
	{
		tmp = align_read(op);
		if(tmp < 0) return tmp;
	}

	if(op->pt != NULL) free(op->pt);
  return 0;
}


#ifndef _FINGERPRINT_MAIN_C_

int main(int argc, char ** argv)
{
  struct ALN_Options op;
  char c;
  int options;

  init_ALN_Options(&op);
  options = 0;
  while( (c=getopt(argc,argv,"i:d:r:s:t:v:V")) >=0)
  {
    switch(c)
    {
      case 'i': op.index = optarg; break;
      case 'd': op.spt = optarg; break;
      case 'r': op.read = optarg; break;
      case 's': op.sam = optarg; break;
      case 't': op.threshold = atoi(optarg); break;
      case 'v': op.verbose = atoi(optarg); break;
      case 'V': fprintf(stdout, "%s\r\n", VERSION); return 0;
      default: return print_help();
    }
  }

  return aln_by_fingerprint(&op);
}

#endif


