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
	op->threshold = 0;

	op->items = 0;
	op->tsize = 0;
	op->pt = NULL;
	op->table = NULL;

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

	if(fread(&header, sizeof(struct SPT_Header), 1, fp) != 1) return -2;
	op->items = header.items;
	op->length = header.length;
	op->interval = header.interval;
	op->band = header.band;

	if(judge_range(header.max) != 0)
	{
		fprintf(stderr, "Fingerprint range overflow \r\n");
		return -3;
	}

	fprintf(stderr, "> Going to read %d fingerprint. len:%d interval:%d band:%d\r\n", \
					op->items, op->length, op->interval, op->band);
	op->pt = (Fingerprint *) malloc(sizeof(Fingerprint) * op->items);
	if(op->pt == NULL)
	{
		fprintf(stderr, "> Memory allocate failed\r\n");
		return -3;
	}

	fseek(fp, sizeof(struct SPT_Header), SEEK_SET);
	if(fread(op->pt, sizeof(Fingerprint), op->items, fp) != op->items)
   	{
	   fprintf(stderr, "Read spt/uspt file failed\r\n");
	   free(op->pt);
	   return -2;
   	}

	//fprintf(stdout, "%d: %d \r\n", op->pt[0].pos, op->pt[1].pos);

	fprintf(stderr, "> Done\r\n");
	fclose(fp);

	if(strcmp(getFileType(op->spt), "uspt") == 0) return 0;

	fp = fopen(op->hash, "rb");
	if(fp == NULL) 
	{
		fprintf(stderr, "Database cannot open:%s\r\n", op->hash);
		free(op->pt); return -2;
	}

	fseek(fp, 0, SEEK_END);
	i = ftell(fp);
	op->tsize = i / sizeof(Hash);
	op->table = (Hash *) malloc(op->tsize * sizeof(Hash));
	fprintf(stderr, "> Going to read hash table (%d items) \r\n", op->tsize);
	fseek(fp, 0, SEEK_SET);
	if(fread(op->table, sizeof(Hash), i, fp) != i)
	{
		fprintf(stderr, "items read: %d \r\n", i);
	   	fclose(fp); free(op->table); free(op->pt); return -2;
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

static u32 get_position(char * name)
{
	int i, j, pos[2];

	j = 0;
	pos[0] = pos[1] = 0;
	for(i=0; i<(int)strlen(name); i++)
	{
		if(name[i] == '_') pos[j++] = i;
		if(j == 2) break;
	}

	if(pos[1] == 0) return 0;
	else return atoi(name+pos[0]+1);

}

static int align_read_conflict(struct ALN_Options * op)
{
	FILE * read, * pac;
	char * buffer, * bbuffer;
	char sn[100];
	int tmp, size;
	u32 i, begin, end;
	u32 score[2];
	u32 pos;
	FType print[4][8], * ptptr;

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
	bbuffer = (char *)malloc(op->length + 1);
	bbuffer[op->length] = 0;

	pos = 0;
	while(1)
	{
		if(feof(read) != 0) break;

		if(read2b_util(read, '>', 0, NULL, 0) < 0) break;;
		tmp = fscanf(read, "%s", sn);
		if(tmp == EOF) break;

		pos = get_position(sn);
		if(pos == 0) break;

		if(read2b_util(read, '\n', 0, NULL, 0) < 0) break;

		if(read2b_util(read, '>', 1, buffer, op->length) != 0) break;

		strncpy(bbuffer, buffer + op->length/2, op->length - op->length/2 + 1);
		strncat(bbuffer, buffer, op->length/2);

		stampFinger8(print[0], buffer, op->length);
		stampFinger8(print[1], bbuffer, op->length);
		
		print[2][0] = print[0][0];
		print[2][1] = print[0][1];
		print[2][2] = print[0][2];
		print[2][3] = print[0][3];
		print[2][4] = print[0][4];
		print[2][5] = print[0][5];
		print[2][6] = print[0][6];
		print[2][7] = print[0][7];

		print[3][0] = print[0][7];
		print[3][1] = print[0][6];
		print[3][2] = print[0][5];
		print[3][3] = print[0][4];
		print[3][4] = print[0][3];
		print[3][5] = print[0][2];
		print[3][6] = print[0][1];
		print[3][7] = print[0][0];

		ptptr = search_position(op, pos);
		if(ptptr == NULL) break;

		size = 8;
		score[0] = estimate(print[2], ptptr, size);
		score[1] = estimate(print[3], ptptr, size);
		if(score[0] > score[1]) score[0] = score[1];
		
		fprintf(stdout, "%lx %lx %lx \r\n", get_key(print[2]),get_key(print[3]), get_key(ptptr));

		tmp = 0;
		for(i=begin; i<end; i++)
		{
			if(score[0] >= estimate(print[2], op->pt[i].print, size) || \
				score[1] >= estimate(print[3], op->pt[i].print, size))
			{
				tmp++;
				fprintf(stdout, "pos: %d, expect: %d\r\n", op->pt[i].pos, pos);
				fprintf(stdout, "%d ", get_key(print[2]) == get_key(op->pt[i].print));
				fprintf(stdout, "%d \r\n", get_key(print[3]) == get_key(op->pt[i].print));
			}
		}
		fprintf(stdout, "Conflicts: %d \r\n", tmp);
		getchar();
	}

	fclose(pac);
	fclose(read);
	free(buffer);
	return 0;
}

static int align_read_debug(struct ALN_Options * op)
{
	FILE * read, * pac;
	char * buffer;
	char sn[100];
	int tmp;
	u32 i, begin, end;
	u32 score;
	u32 pos;
	Alignment align;
	FType print[4][FPSize], * ptptr;

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

		pos = get_position(sn);
		if(pos == 0) break;

		if(read2b_util(read, '\n', 0, NULL, 0) < 0) break;

		if(read2b_util(read, '>', 1, buffer, op->length) != 0) break;

		stampFinger(print[0], buffer, op->length);
		reverseFinger(print[0], print[1]);
		
		ptptr = search_position(op, pos);
		if(ptptr == NULL) break;

		align.score = estimate(print[0], ptptr, FPSize);
		score = estimate(print[1], ptptr, FPSize);
		if(score < align.score) align.score = score;

		tmp = 0;
		for(i=begin; i<end; i++)
		{
			if(align.score >= estimate(print[0], op->pt[i].print, FPSize) || \
				align.score >= estimate(print[1], op->pt[i].print, FPSize))
			{
				tmp++;
			}
		}
		fprintf(stdout, "Conflicts: %d \r\n", tmp);
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
	FType print[2][FPSize];
	Hash hash, * hptr;

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

		stampFinger8(print[0], buffer, op->length);

		// forward-forward mapping
		hash.key = get_key(print[0]);
		hptr = (Hash *)bsearch(&hash, op->table, op->tsize, sizeof(Hash), compare_hash);

		begin = hptr->beg; end = hptr->end;
		for(i=begin; i<end; i++)
		{
			score = estimate(print[0], op->pt[i].print, FPSize);
		}

		// forward-reverse mapping
		reverseFinger(print[0], print[1]);
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

	if((op->verbose & 0x07) == 0x07)
	{
		tmp = align_read_conflict(op);
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
	if(op->table != NULL) free(op->table);
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


