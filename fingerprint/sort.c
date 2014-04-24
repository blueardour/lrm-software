//  ******************************************************************//
//  author: chenp
//  description: sort fingerprint database of the reference
//  version: 1.0
//	date: 2014-03-12
//	date: 2014-04-22 ver2.0
//  ******************************************************************//


#include "sort.h"

#define VERSION "2.0"
#define PROGRAM "fblra sort"


void init_Sort_Options(struct Sort_Options * op)
{
	op->verbose = -1;
	op->prefix = NULL;
	op->pattern = NULL;
	op->uspt = NULL;
	op->spt = NULL;
	op->info = NULL;
	op->hash = NULL;
}

static int print_help()
{
	fprintf(stdout, "%s ", PROGRAM);
	fprintf(stdout, "ver (%s):\r\n", VERSION);
	fprintf(stdout, "  -v[erbose] num\r\n");
	fprintf(stdout, "  -V(ersion)\r\n");
	fprintf(stdout, "  -u[spt] str *\r\n");
	fprintf(stdout, "  -p[attern] str \r\n");
	fprintf(stdout, "  -s[pt] str\r\n");
	return 0;
}

static void dump_Options(struct Sort_Options * op)
{
	fprintf(stdout, "%s ", PROGRAM);
	fprintf(stdout, "ver (%s):\r\n", VERSION);
	fprintf(stdout, "  -verbose 0x%02x\r\n", op->verbose);
	fprintf(stdout, "  -pattern %s\r\n", op->pattern);
	fprintf(stdout, "  -uspt %s\r\n", op->uspt);
	fprintf(stdout, "  -spt %s\r\n", op->spt);
	fprintf(stdout, "  -info %s\r\n", op->info);
	fprintf(stdout, "  -hash %s\r\n", op->hash);
}

static int format_Options(struct Sort_Options * op)
{ 
	if(op->uspt == NULL) return -1;

	if(FPSize > 10) return -2;

	if(op->spt == NULL)
	{
		op->spt = (char *)malloc(strlen(op->uspt) + 5);
		strcpy(op->spt, op->uspt);
		strcat(op->spt, ".spt");
	}

	op->info = (char *)malloc(strlen(op->spt) + 5);
	strcpy(op->info, op->spt);
	strcat(op->info, ".inf");

	op->hash = (char *)malloc(strlen(op->spt) + 5);
	strcpy(op->hash, op->spt);
	strcat(op->hash, ".hsh");

	if(op->verbose & 0x80) op->verbose = 0x80;

	return 0;
}


static int build_hash(struct Index_Key * index, u32 size, struct Sort_Options * op)
{
	FILE * info, * hash;
	struct Index_Hash * table;
	u32 table_size;
	u32 i, j;

	info = fopen(op->info, "wb");
	if(info == NULL)
	{
		fprintf(stderr, "Cannot create file:%s\r\n", op->info);
		return -2;
  	}

	hash = fopen(op->hash, "wb");
	if(hash == NULL)
	{
		fprintf(stderr, "Cannot create file:%s\r\n", op->hash);
		return -2;
  	}

	table_size = 60000;
	table = (struct Index_Hash *) malloc(sizeof(struct Index_Hash) * table_size);
	if(table == NULL)
	{
		fprintf(stderr, "memories malloc failed \r\n");
		return -5;
	}

	table[0].key = index[0].key;
	table[0].left = 0;
	for(i=0,j=0; i<size; i++)
	{
		fprintf(info, "%016lx\r\n", index[i].key);

		if(table[j].key != index[i].key)
		{
			table[j++].right = i;

			if(j == table_size)
			{
				table_size += 60000;
				table = (struct Index_Hash *)realloc(table, sizeof(struct Index_Hash) * table_size);
			}
			table[j].left = i;
			table[j].key = index[i].key;
		}
	}
	table[j++].right = i;
	
	fprintf(stdout, "Hash items: %d \r\n", j);

	if(fwrite(table, sizeof(struct Index_Hash), j, hash) != j)
	{
		fprintf(stderr, "hash file write failed \r\n");
		return -5;
	}

	free(table);
	fclose(hash);
	fclose(info);
	return 0;
}

static int compare_key(const void * a, const void * b)
{
	return ((struct Index_Key *) a)->key - ((struct Index_Key *) b)->key;
}


int sort_fingerprint(struct Sort_Options * op)
{
	char c;
	FILE * fp, * database;
	Fingerprint pt;
	u32 i;
	struct SPT_Header header;
	struct Index_Key * index;
	
	fp = database = NULL;

	if(format_Options(op) < 0) return -1;

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

		if(fread(&header, sizeof(struct SPT_Header), 1, fp) != 1)
		{
			fprintf(stderr, "Loading uspt file failed\r\n");
			fclose(fp);
			return -3;
		}
		else
		{
			if(sizeof(Fingerprint) != (FPSize * sizeof(FType) + sizeof(u32)))
			{
				fprintf(stderr, "Fingerprint size not match %ld \r\n", sizeof(Fingerprint));
				return -1;
			}

			fprintf(stdout, "> %d items read. len:%d interval:%d band:%d\r\n", \
						   header.items, header.length, header.interval, header.band);

			if(op->verbose & 0x02)
			{
				fprintf(stdout, " Is that correct? [y/n]");
				do { c=getchar(); } while(c != 'y' && c!= 'n');
				if(c == 'n') { fclose(fp); return 0; }
			}
		}

		index = (struct Index_Key *) malloc(sizeof(struct Index_Key) * header.items);
		if(index == NULL)
		{
			fprintf(stderr, "Malloc memories failed \r\n");
			fclose(fp);
			return -5;
		}

		if(fseek(fp, sizeof(struct SPT_Header), SEEK_SET) == -1)
		{
			fprintf(stderr, "Fseek uspt/spt file error\r\n");
			fclose(fp);
			return -4;
		}

		for(i=0; i<header.items; i++)
		{
			if(fread(&pt, sizeof(pt), 1, fp) != 1)
			{
				fprintf(stderr, "Read fingerprint error\r\n");
				fclose(fp); free(index);
				return -4;
			}

			index[i].key = getKey(pt.print);
			index[i].i = i;
		}

		qsort(index, header.items, sizeof(struct Index_Key), compare_key);

		database = fopen(op->spt, "wb");
  		if(database == NULL)
  		{
  		  fprintf(stderr, "Cannot create file:%s\r\n", op->spt);
  		  return -2;
  		}

		fwrite(&header, sizeof(struct SPT_Header), 1,  database);

		for(i=0; i<header.items; i++)
		{
			if(fseek(fp, sizeof(struct SPT_Header) + index[i].i * sizeof(pt), SEEK_SET) == -1)
			{
				free(index); fclose(fp); fclose(database); return -6;
			}
			if(fread(&pt, sizeof(pt), 1, fp) != 1)
			{
				free(index); fclose(fp); fclose(database); return -6;
			}

			// debug
			//fprintf(stderr, "pos: %d %d \r\n", pt.pos, index[i].i);

			if(fseek(database, sizeof(struct SPT_Header) + i * sizeof(pt), SEEK_SET) == -1)
			{
				free(index); fclose(fp); fclose(database); return -6;
			}
			if(fwrite(&pt, sizeof(pt), 1, database) != 1)
			{
				free(index); fclose(fp); fclose(database); return -6;
			}
		}

		build_hash(index, header.items, op);

		free(index);
		fclose(fp);
		fclose(database);
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
  while( (c=getopt(argc,argv,"u:p:s:v:V")) >=0)
  {
    switch(c)
    {
      case 'u': options++; op.uspt = optarg; break;
      case 'p': op.pattern = optarg; break;
      case 's': op.spt = optarg; break;
      case 'v': op.verbose = atoi(optarg); break;
      case 'V': fprintf(stdout, "%s\r\n", VERSION); return 0;
      default: return print_help();
    }
  }

  if(options < 1) return print_help();

  return sort_fingerprint(&op);
}

#endif

