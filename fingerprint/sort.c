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
	u08 verbose;
	u32 length, interval, band;
	u32 items;
	char * prefix;
	char * pattern;
	char * si;
	char * uspt;
	char * spt;
};

void init_Sort_Options(struct Sort_Options * op)
{
	op->verbose = 3;
	op->prefix = NULL;
	op->pattern = NULL;
	op->si = NULL;
	op->uspt = NULL;
	op->spt = NULL;
}

static int print_help()
{
	fprintf(stdout, "%s ", PROGRAM);
	fprintf(stdout, "ver (%s):\r\n", VERSION);
	fprintf(stdout, "  -v(erbose):\r\n");
	fprintf(stdout, "  -u[spt] str\r\n");
	fprintf(stdout, "  -p(attern) str\r\n");
	fprintf(stdout, "  -V(ersion)\r\n");
	return 0;
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

int sort_fingerprint(struct Sort_Options * op)
{
	FILE * database;
	int tmp;
	FType finger[8], print[8];
	u32 i, j;
	
	format_Options(op);
	fp = database = NULL;
	ptr = NULL;
	tmp = 0;

	fprintf(stdout, "===> Dump parameters...%d\r\n", op->verbose & 0x80);
	if(op->verbose & 0x80)
	{
		dump_Options(op);
	}

	fprintf(stdout, "===> Sorting uspt file...%d\r\n", op->verbose & 0x01);
	if(op->verbose & 0x01)
	{
		database = fopen(op->uspt, "rb");
  	if(database == NULL)
  	{
  	  fprintf(stderr, "Database cannot open:%s\r\n", op->uspt);
  	  return -2;
  	}

		tmp = 0;
		if(fread(&op->items, sizeof(op->items), 1, fp) != 1) tmp++;
		if(fread(&op->length, sizeof(op->length), 1, fp) != 1) tmp++;
		if(fread(&op->interval, sizeof(op->interval), 1, fp) != 1) tmp++;
		if(fread(&op->band, sizeof(op->band), 1, fp) != 1) tmp++;

		if(tmp != 0)
		{
			fprintf(stderr, "");
			return -3;
		}

		fprintf(stdout, "");


		for(i=0; i<op->items; i++)
		{
			//if(fread(&tmp, sizeof(tmp), 1, fp);
			//fwrite(&j, sizeof(j), 1, fp);
			//fwrite(print, sizeof(FType), 8, fp);
		}
	} // if

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

