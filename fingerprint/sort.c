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
	char * prefix;
	char * pattern;
	char * uspt;
	char * spt;
};

void init_Sort_Options(struct Sort_Options * op)
{
	op->verbose = 3;
	op->prefix = NULL;
	op->pattern = NULL;
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
}

static void format_Options(struct Sort_Options * op)
{ 
	int len;

	len = strlen(op->prefix);

	if(op->uspt == NULL)
	{
		op->uspt = (char *)malloc(len+6);
		strcpy(op->uspt, op->prefix);
		strcat(op->uspt, ".uspt");
	}

	if(op->spt == NULL)
	{
		op->spt = (char *)malloc(len+10);

		strcpy(op->spt, op->prefix);
		strcat(op->spt, "-");
		strncat(op->spt, op->pattern, 4);
		strcat(op->spt, ".spt");
	}

	if(op->verbose & 0x80) op->verbose = 0x80;
}

int sort_fingerprint(struct Sort_Options * op)
{
	int tmp, shift, offset;
	FILE * fp, * database;
	FType print[8], max[8], min[8];
	u32 items;
	u32 count;
	u32 length, band, interval;
	u32 * ptptr;
	u32 pos;
	u32 i;
	
	if(op->pattern == NULL || op->prefix == NULL) return -1;
	format_Options(op);
	fp = database = NULL;
	ptptr = NULL;
	tmp = 0;

	fprintf(stdout, "===> Dump parameters...%d\r\n", op->verbose & 0x80);
	if(op->verbose & 0x80)
	{
		dump_Options(op);
	}

	fprintf(stdout, "===> Sort uspt file...%d\r\n", op->verbose & 0x01);
	if(op->verbose & 0x01)
	{
		fp = fopen(op->uspt, "rb");
  	if(fp == NULL)
  	{
  	  fprintf(stderr, "Database cannot open:%s\r\n", op->uspt);
  	  return -2;
  	}

		tmp = 0;
		if(fread(&items, sizeof(items), 1, fp) != 1) tmp++;
		if(fread(max, sizeof(FType), 8, fp) != 8) tmp++;
		if(fread(min, sizeof(FType), 8, fp) != 8) tmp++;
		if(fread(&length, sizeof(length), 1, fp) != 1) tmp++;
		if(fread(&interval, sizeof(interval), 1, fp) != 1) tmp++;
		if(fread(&band, sizeof(band), 1, fp) != 1) tmp++;

		if(tmp != 0)
		{
			fprintf(stderr, "Loading uspt file failed\r\n");
			fclose(fp);
			return -3;
		}
		else
		{
			fprintf(stdout, "> %d items read. len:%d interval:%d band:%d\r\n", items, length, interval, band);
		}

		shift = 0;
		for(tmp=0; tmp<strlen(op->pattern); tmp++)
		{
			if(op->pattern[tmp] == 'A')	shift = 0;
			else if(op->pattern[tmp] == 'C') shift = 1;
			else if(op->pattern[tmp] == 'G') shift = 2;
			else if(op->pattern[tmp] == 'T') shift = 3;
			else continue;
			break;
		}

		fprintf(stdout, "> Major Index:%c, max:%d, min:%d\r\n", op->pattern[tmp], max[shift], min[shift]);

		ptptr = (u32 *) malloc(sizeof(u32)*(max[shift]-min[shift]+1));
		memset(ptptr, 0, sizeof(u32)*(max[shift]-min[shift]+1));

		offset = sizeof(u32)*4 + sizeof(FType)*16 + 20;
		if(fseek(fp, offset, SEEK_SET) != 0)
		{
			fprintf(stderr, "Fseek uspt/spt file error\r\n");
			fclose(fp);
			return -4;
		}

		for(i=0; i<items; i++)
		{
			if(fread(&pos, sizeof(pos), 1, fp) != 1)
			{
				fprintf(stderr, "Read fingerprint error\r\n");
				fclose(fp);
				return -4;
			}
			if(fread(print, sizeof(FType), 8, fp) != 8)
			{
				fprintf(stderr, "Read fingerprint error\r\n");
				fclose(fp);
				return -4;
			}
			if(print[shift] >= min[shift] && print[shift] <= max[shift])
				ptptr[print[shift] - min[shift]]++;
			else
			{
				fprintf(stderr, "Unexpected fingerprint, cur:%d\r\n", print[shift]);
				fclose(fp);
				return -4;
			}
		}

		for(i=1; i<max[shift]-min[shift]+1; i++)
		{
			ptptr[i] = ptptr[i] + ptptr[i-1];
		}

		database = fopen(op->spt, "wb");
  	if(database == NULL)
  	{
  	  fprintf(stderr, "Cannot create file:%s\r\n", op->spt);
  	  return -2;
  	}

		fwrite(&items, sizeof(items), 1, database);
		fwrite(max, sizeof(FType), 8, database);
		fwrite(min, sizeof(FType), 8, database);
		fwrite(&length, sizeof(length), 1, database);
		fwrite(&interval, sizeof(interval), 1, database);
		fwrite(&band, sizeof(band), 1, database);

		fwrite(op->pattern, 1, 4, database);

		tmp = sizeof(u32)+ 8*sizeof(FType);
		if(fseek(database, offset + items * tmp, SEEK_SET) != 0)
		{
			fprintf(stderr, "Fseek in spt file error.(%d)\r\n", offset + items * tmp);
			fclose(fp);
			fclose(database);
			return -5;
		}

		fseek(fp, offset, SEEK_SET);
		for(i=0; i<items; i++)
		{
			if(fread(&pos, sizeof(pos), 1, fp) != 1)
			{
				fprintf(stderr, "Read fingerprint error\r\n");
				fclose(fp);
				return -4;
			}
			if(fread(print, sizeof(FType), 8, fp) != 8)
			{
				fprintf(stderr, "Read fingerprint error\r\n");
				fclose(fp);
				return -4;
			}

			count = --ptptr[print[shift] - min[shift]];
			if(count >= 0 && fseek(database, offset + count*tmp, SEEK_SET) == 0)
			{
				fwrite(&pos, sizeof(pos), 1, database);
				fwrite(print, sizeof(FType), 8, database);
			}
			else
			{
				fprintf(stderr, "Re-squence uspt file error \r\n");
				fclose(fp);
				fclose(database);
				return -6;
			}
		}
		fclose(fp);
		fclose(database);
	}

	fprintf(stdout, "===> Count conflicts file...%d\r\n", op->verbose & 0x02);
	if(op->verbose & 0x02)
	{
	}

  return 0;
}


#ifndef _FINGERPRINT_MAIN_C_

int main(int argc, char ** argv)
{
  struct Sort_Options op;
  char c;
  int options;

  init_Sort_Options(&op);
  options = 0;
  while( (c=getopt(argc,argv,"p:u:v:V")) >=0)
  {
    switch(c)
    {
      case 'u': options++; op.prefix = optarg; break;
      case 'p': options++; op.pattern = optarg; break;
      case 'v': op.verbose = atoi(optarg); break;
      case 'V': fprintf(stdout, "%s\r\n", VERSION); return 0;
      default: return print_help(getFileName(argv[0]));
    }
  }

  if(options < 2) return print_help(getFileName(argv[0]));

  return sort_fingerprint(&op);
}

#endif

