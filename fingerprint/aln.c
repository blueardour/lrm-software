//  ******************************************************************//
//  author: chenp
//  description: aligner 
//  version: 1.0
//	date: 2014-03-20
//  ******************************************************************//


#include "aln.h"

#define VERSION "1.0"
#define PROGRAM "fblra aln"

#define DEBUG 2


static int print_help()
{
	fprintf(stderr, "%s ", PROGRAM);
	fprintf(stderr, "ver (%s):\r\n", VERSION);
	fprintf(stderr, "  -V(ersion)\r\n");
	fprintf(stderr, "  -v(erbose)\r\n");
	fprintf(stderr, "  -d(atabase) str *\r\n");
	fprintf(stderr, "  -s(pt) str *\n");
	fprintf(stderr, "  -r(ead) str *\r\n");
	fprintf(stderr, "  -o[utput] str *\r\n");
	return 0;
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
	op->pac = NULL;
	op->spt = NULL;
	op->read = NULL;
	op->sam = NULL;
	op->si = NULL;
}

static int format_Options(struct ALN_Options * op)
{ 
	int len;
	char * ptr;

	if(op->prefix == NULL) return -1;

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

	len = strlen(op->prefix);
	op->pac = (char *)malloc(len + 6);
	op->si = (char *)malloc(len + 6);

	strcpy(op->pac, op->prefix);
	strcpy(op->si, op->prefix);

	strcat(op->pac, ".pac");
	strcat(op->si, ".si");

	if(op->verbose & 0x80) op->verbose = 0x80;
	return 0;
}

static void dump_Options(struct ALN_Options * op)
{
	fprintf(stderr, "%s ", PROGRAM);
	fprintf(stderr, "ver (%s):\r\n", VERSION);
  fprintf(stderr, "  -reference %s\r\n", op->prefix);
  fprintf(stderr, "  -spt %s\r\n", op->spt);
  fprintf(stderr, "  -read %s\r\n", op->read);
  fprintf(stderr, "  -sam %s\r\n", op->sam);
  fprintf(stderr, "  -pac %s\r\n", op->pac);
  fprintf(stderr, "  -si %s\r\n", op->si);
  fprintf(stderr, "  -threshold %d\r\n", op->threshold);
  fprintf(stderr, "  -verbose 0x%02x\r\n", op->verbose);
}

static int load_spt(struct ALN_Options * op)
{
	FILE * fp;
	Fingerprint lpt, spt;
	int offset;
	u32 i;

	fp = fopen(op->spt, "rb");
	if(fp == NULL) 
	{
		fprintf(stderr, "Database cannot open:%s\r\n", op->spt);
		return -2;
	}

	fseek(fp, 20, SEEK_SET);
	if(fread(&op->items, sizeof(op->items), 1, fp) != 1) return -2;
	if(fread(lpt.print, sizeof(FType), FPSize, fp) != FPSize) return -2;
	if(fread(spt.print, sizeof(FType), FPSize, fp) != FPSize) return -2;
	if(fread(&op->length, sizeof(op->length), 1, fp) != 1) return -2;
	if(fread(&op->interval, sizeof(op->interval), 1, fp) != 1) return -2;
	if(fread(&op->band, sizeof(op->band), 1, fp) != 1) return -2;
	if(fread(op->pattern, 1, 4, fp) != 4) return -2;

	fprintf(stderr, "> Going to read %d fingerprint. len:%d interval:%d band:%d\r\n", \
					op->items, op->length, op->interval, op->band);
	op->pt = (Fingerprint *) malloc(sizeof(Fingerprint) * op->items);
	if(op->pt == NULL)
	{
		fprintf(stderr, "> Memory allocate failed\r\n");
		return -3;
	}

	offset = sizeof(FType)*FPSize*2 + sizeof(u32)*4 + 40;
	fseek(fp, offset, SEEK_SET);
	for(i=0; i<op->items; i++)
	{
		if(fread(op->pt + i, sizeof(Fingerprint), 1, fp) != 1) return -2;

#if(DEBUG == 1)
		if(value(op->pt[i].pos, 79154047) < op->interval * 2) printf("pos: %d \r\n", op->pt[i].pos);
#endif
	}

	fprintf(stderr, "> Done\r\n");
	fclose(fp);
	return 0;
}


static u32 search_position(struct ALN_Options * op, u32 pos)
{
	u32 i;
	for(i=0; i<op->items; i++)
	{
		if(value(op->pt[i].pos, pos) <= op->interval)
		{
			return op->pt[i].pos;
		}
	}
	return 0;
}

/*
static u32 search_range(struct PT * list, u32 len, u32 * beg, u32 * end, FType spt, FType lpt)
{
	return 0;
}
*/

static int align_read(struct ALN_Options * op)
{
	FILE * read;
	char * buffer;
	char sn[100];
	int tmp;
	u32 i, begin, end;
	FType print[FPSize*2], * ptptr;

	read = fopen(op->read, "r");
	if(read == NULL) 
	{
		fprintf(stderr, "File cannot open:%s\r\n", op->read);
		return -2;
	}

	/*
	pac = fopen(op->pac, "r");
	if(pac == NULL) 
	{
		fprintf(stderr, "File cannot open:%s\r\n", op->pac);
		return -2;
	}

	sam = fopen(op->sam, "r");
	if(read == NULL) 
	{
		fprintf(stderr, "Cannot create file:%s\r\n", op->sam);
		return -2;
	}
	*/

	begin = 0;
	end = op->items;
	buffer = (char *)malloc(op->length + 1);
	buffer[op->length] = 0;
	while(1)
	{
		if(feof(read) != 0) break;

#if(DEBUG != 3)
		if(read2b_util(read, '>', 0, NULL, 0) < 0) break;;
		tmp = fscanf(read, "%s", sn);

		if(read2b_util(read, '\n', 0, NULL, 0) < 0) break;

		if(read2b_util(read, '>', 1, buffer, op->length) != 0) break;
#endif

		stampFinger(print, buffer, op->length);

#if(DEBUG == 1)
		u32 debug;
		fprintf(stderr, "SN:%s\r\n", sn);
		ptptr = print;
		for(tmp=0; tmp<FPSize; tmp++) fprintf(stderr, "%d ", ptptr[tmp]);
		fprintf(stderr, "\r\n");
		
		fprintf(stderr, "SN[%d]\r\n", op->pt[0].pos);
		ptptr = op->pt[0].print;
		for(tmp=0; tmp<FPSize; tmp++) fprintf(stderr, "%d ", ptptr[tmp]);
		fprintf(stderr, "\r\n");
		
		for(i=begin; i<end; i++)
		{
			debug = estimate(print, op->pt[i].print, FPSize);
			if(debug < 16000)
				fprintf(stdout, "%d : %d\r\n", op->pt[i].pos, debug);
		}
		break;
#endif

#if(DEBUG == 3 || DEBUG == 2)
		u32 debug;
		fprintf(stdout, "SN:%s\r\n", sn);
		
		ptptr = print;
		fprintf(stdout, "Print: %d %d %d %d\r\n", ptptr[0], ptptr[1], ptptr[2], ptptr[3]);
		for(i=begin; i<end; i++)
		{
			debug = estimate(print, op->pt[i].print, FPSize);
			if(debug < op->threshold)
				fprintf(stdout, "Forward: [%d]-> %d : %d\r\n", i, op->pt[i].pos, debug);

			debug = estimateReverse(print, op->pt[i].print, FPSize);
			if(debug < op->threshold)
				fprintf(stdout, "Reverse: [%d]-> %d : %d\r\n", i, op->pt[i].pos, debug);

			/*
			if(value(op->pt[i].pos, 115717560) <= op->interval)
			{
				fprintf(stdout, "Pos: %d\r\n", op->pt[i].pos);
				estimate_debug(print, op->pt[i].print, FPSize);
				estimateReverse_debug(print, op->pt[i].print, FPSize);
			}

			if(i==626109 || i==626110 || i==10371923 || i==21108530)
			{
				ptptr = print;
				fprintf(stdout, "Print: %d %d %d %d\r\n", ptptr[0], ptptr[1], ptptr[2], ptptr[3]);
				ptptr = op->pt[i].print;
				fprintf(stdout, "Print: %d %d %d %d\r\n", ptptr[0], ptptr[1], ptptr[2], ptptr[3]);
				fprintf(stdout, "[%d]-> %d : %d\r\n", i, op->pt[i].pos, estimate_debug(print + 4, op->pt[i].print, FPSize));
			}
			*/
		}

#endif

		getchar();
	}

	fclose(read);
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

	if(op->verbose & 0x04)
	{
		fprintf(stdout, "pos: %d \r\n", search_position(op, 148131972));
		op->verbose = 0x00;
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
  while( (c=getopt(argc,argv,"t:o:r:s:d:v:V")) >=0)
  {
    switch(c)
    {
      case 'o': options |= 0x01; op.sam = optarg; break;
      case 'r': options |= 0x02; op.read = optarg; break;
      case 's': options |= 0x04; op.spt = optarg; break;
      case 'd': options |= 0x08; op.prefix = optarg; break;
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


