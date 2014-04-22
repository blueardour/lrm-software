//  ******************************************************************//
//  author: chenp
//  description: aligner 
//  version: 1.0
//	date: 2014-03-20
//  ******************************************************************//


#include "aln.h"

#define VERSION "2.0"
#define PROGRAM "fblra aln"


static int print_help()
{
	fprintf(stderr, "%s ", PROGRAM);
	fprintf(stderr, "ver (%s):\r\n", VERSION);
	fprintf(stderr, "  -V(ersion)\r\n");
	fprintf(stderr, "  -v(erbose)\r\n");
	fprintf(stderr, "  -r(eference) str *\r\n");
	fprintf(stderr, "  -q(urey) str *\r\n");
	fprintf(stderr, "  -s(pt) str *\n");
	fprintf(stderr, "  -o[utput] str *\r\n");
	fprintf(stderr, "  -t(hreshold)\r\n");
	return 0;
}

static void dump_Options(struct ALN_Options * op)
{
	fprintf(stderr, "%s ", PROGRAM);
	fprintf(stderr, "ver (%s):\r\n", VERSION);
	fprintf(stderr, "  -verbose 0x%02x\r\n", op->verbose);
	fprintf(stderr, "  -reference %s\r\n", op->prefix);
	fprintf(stderr, "  -read %s\r\n", op->read);
	fprintf(stderr, "  -pac %s\r\n", op->pac);
	fprintf(stderr, "  -si %s\r\n", op->si);
	fprintf(stderr, "  -spt %s\r\n", op->spt);
	fprintf(stderr, "  -hash %s\r\n", op->hash);
	fprintf(stderr, "  -sam %s\r\n", op->sam);
	fprintf(stderr, "  -threshold %d\r\n", op->threshold);
}

void init_ALN_Options(struct ALN_Options * op)
{
	op->verbose = -1;
	op->length = 0;
	op->interval = 0;
	op->band = 0;
	op->items = 0;

	op->threshold = 0;
	op->pt = NULL;

	op->prefix = NULL;
	op->dir = NULL;

	op->hash = NULL;
	op->si = NULL;
	op->pac = NULL;
	op->spt = NULL;
	op->read = NULL;
	op->sam = NULL;
}

static int format_Options(struct ALN_Options * op)
{ 
	int len;
	char * ptr;

	if(op->spt == NULL || op->read == NULL) return -1;

	ptr = op->spt + strlen(op->spt) - 3;
	if(strcmp(ptr, "spt") != 0)
	{
		fprintf(stderr, "File type not supported: %s\r\n", op->spt);
		return -2;
	}

	ptr = getFileType(op->read);
	if(strcmp(ptr, "fa") != 0 && strcmp(ptr, "fasta") != 0)
	{
		fprintf(stderr, "File type not supported: %s\r\n", op->read);
		return -2;
	}

	if(op->prefix == NULL) return -1;

	len = 0;
	if(op->dir != NULL) len = strlen(op->dir);
	len += strlen(op->prefix);

	op->pac = (char *)malloc(len + 6);
	op->si = (char *)malloc(len + 6);

	if(op->dir != NULL)
	{
		strcpy(op->pac, op->dir);
		strcpy(op->si, op->dir);

		strcat(op->pac, op->prefix);
		strcat(op->si, op->prefix);
	}
	else
	{
		strcpy(op->pac, op->prefix);
		strcpy(op->si, op->prefix);
	}

	strcat(op->pac, ".pac");
	strcat(op->si, ".si");

	op->hash = (char *)malloc(strlen(op->spt) + 5);
	strcpy(op->hash, op->spt);
	strcat(op->hash, ".hsh");

	if(op->verbose & 0x80) op->verbose = 0x80;
	return 0;
}

static int load_spt(struct ALN_Options * op)
{
	FILE * fp;
	u32 i;
	struct SPT_Header header;

	fp = fopen(op->spt, "rb");
	if(fp == NULL) 
	{
		fprintf(stderr, "Database cannot open:%s\r\n", op->spt);
		return -2;
	}

	if(fread(&header, sizeof(header), 1, fp) != 1) return -2;
	op->items = header.items;
	op->length = header.length;
	op->interval = header.length;
	op->band = header.band;

	fprintf(stderr, "> Going to read %d fingerprint. len:%d interval:%d band:%d\r\n", \
					op->items, op->length, op->interval, op->band);
	op->pt = (Fingerprint *) malloc(sizeof(Fingerprint) * op->items);
	if(op->pt == NULL)
	{
		fprintf(stderr, "> Memory allocate failed\r\n");
		return -3;
	}

	fseek(fp, sizeof(struct SPT_Header), SEEK_SET);
	for(i=0; i<op->items; i++)
	{
		if(fread(op->pt + i, sizeof(Fingerprint), 1, fp) != 1) { free(op->pt); return -2; }
	}

	fprintf(stderr, "> Done\r\n");
	fclose(fp);

	fp = fopen(op->hash, "rb");
	if(fp == NULL) 
	{
		fprintf(stderr, "Database cannot open:%s\r\n", op->hash);
		free(op->pt); return -2;
	}

	fseek(fp, 0, SEEK_END);
	i = ftell(fp);
	i = i / 16;
	op->index = (struct Index_Hash *) malloc(i * sizeof(struct Index_Hash));
	fprintf(stderr, "> Going to read hash table (%d items) \r\n", i);
	if(fread(op->index, sizeof(struct Index_Hash), i, fp) != i)
	{
		fclose(fp); free(op->index); free(op->pt); return -2;
	}
	fprintf(stderr, "> Done\r\n");

	return 0;
}


static FType * search_position(struct ALN_Options * op, u32 pos)
{
	u32 i;
	for(i=0; i<op->items; i++)
	{
		if(value(op->pt[i].pos, pos) <= op->interval)
		{
			return op->pt[i].print;
		}
	}
	return NULL;
}

static u32 getPosition(char * name)
{
	int i, j, pos[2];

	j = 0;
	pos[0] = pos[1] = 0;
	for(i=0; i<strlen(name); i++)
	{
		if(name[i] == '_') pos[j++] = i;
		if(j == 2) break;
	}

	if(pos[1] == 0) return 0;
	else return atoi(name+pos[0]+1);

}

static int align_read_debug(struct ALN_Options * op)
{
	FILE * read, * pac;
	char * buffer;
	char sn[100];
	int tmp;
	u32 i, begin, end;
	u32 score[2];
	u32 pos;
	struct cal cals;
	FType print[FPSize*2];

	read = fopen(op->read, "r");
	if(read == NULL) 
	{
		fprintf(stderr, "File cannot open:%s\r\n", op->read);
		return -2;
	}

	pac = fopen(op->pac, "r");
	if(pac == NULL) 
	{
		fprintf(stderr, "File cannot open:%s\r\n", op->pac);
		return -2;
	}

	begin = 0;
	end = op->items;

	buffer = (char *)malloc(op->length + 1);
	buffer[op->length] = 0;

	pos = 0;
	while(1)
	{
		if(feof(read) != 0) break;

		if(read2b_util(read, '>', 0, NULL, 0) < 0) break;;
		tmp = fscanf(read, "%s", sn);
		if(tmp == EOF) break;

		pos = getPosition(sn);
		if(pos == 0) break;

		if(read2b_util(read, '\n', 0, NULL, 0) < 0) break;

		if(read2b_util(read, '>', 1, buffer, op->length) != 0) break;

		stampFinger(print, buffer, op->length);
		
		score[0] = estimate(print, search_position(op, pos), FPSize);
		score[1] = estimateReverse(print, search_position(op, pos), FPSize);
		if(score[1] > score[0])
		{
			cals.score = score[0];
			cals.orient = 0;
		}
		else
		{
			cals.score = score[1];
			cals.orient = 0;
		}

		for(i=begin; i<end; i++)
		{
			score[0] = estimate(print, op->pt[i].print, FPSize);
			score[1] = estimateReverse(print, op->pt[i].print, FPSize);
			if(cals.score >= score[0] || cals.score >= score[1]) 
			{
				fprintf(stdout, "%d\t%d\t%d\t%s\r\n", op->pt[i].pos, score[0], score[1], sn);
			}
		}

		getchar();
	}

	fclose(pac);
	fclose(read);
	free(buffer);
	return 0;
}

static int align_read(struct ALN_Options * op)
{
	FILE * read, * sam;
	char * buffer;
	char sn[100];
	int tmp;
	u32 i, begin, end;
	u32 score;
	Alignment align;
	FType print[FPSize];
	Index_Hash hash, * hptr;

	read = fopen(op->read, "r");
	if(read == NULL) 
	{
		fprintf(stderr, "File cannot open:%s\r\n", op->read);
		return -2;
	}

	sam = fopen(op->sam, "w");
	if(sam == NULL) 
	{
		fprintf(stderr, "Cannot create file:%s\r\n", op->sam);
		return -2;
	}

	begin = 0;
	end = op->items;
	buffer = (char *)malloc(op->length + 1);
	buffer[op->length] = 0;
	while(1)
	{
		if(feof(read) != 0) break;

		if(read2b_util(read, '>', 0, NULL, 0) < 0) break;;
		tmp = fscanf(read, "%s", sn);
		if(tmp == EOF) break;

		if(read2b_util(read, '\n', 0, NULL, 0) < 0) break;

		if(read2b_util(read, '>', 1, buffer, op->length) != 0) break;

		stampFinger(print, buffer, op->length);

		// forward-forward mapping
		hash.key = getKey(print);
		hptr = (Index_Hash *)bsearch(&hash, op->index, op->size, sizeof(Index_Hash), compare_hash);

		begin = hptr->left; end = hptr->right;
		for(i=begin; i<end; i++)
		{
		}

		// forward-reverse mapping




	}

	fclose(sam);
	fclose(read);
	free(buffer);
	return 0;
}

int aln_by_fingerprint(struct ALN_Options * op)
{
	int tmp;

	tmp = format_Options(op);
	if(tmp < 0) return tmp;

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

	if((op->verbose & 0x05) == 0x05)
	{
		tmp = align_read_debug(op);
		if(tmp < 0) return tmp;
	}

	fprintf(stdout, "===> Align reads...%d\r\n", (op->verbose & 0x03) == 0x03);
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
  u08 options;

  options = 0;
  init_ALN_Options(&op);
  while( (c=getopt(argc,argv,"o:q:s:r:t:d:v:V")) >=0)
  {
    switch(c)
    {
      case 'o': options |= 0x01; op.sam = optarg; break;
      case 'q': options |= 0x02; op.read = optarg; break;
      case 's': options |= 0x04; op.spt = optarg; break;
      case 'r': options |= 0x08; op.prefix = optarg; break;
      case 'd': options |= 0x00; op.dir = optarg; break;
      case 't': options |= 0x00; op.threshold = atoi(optarg); break;
      case 'v': options |= 0x00; op.verbose = atoi(optarg); break;
      case 'V': fprintf(stdout, "%s\r\n", VERSION); return 0;
      default: return print_help();
    }
  }

	if((options & 0x0f) != 0x0f) return print_help();

  return aln_by_fingerprint(&op);
}

#endif


