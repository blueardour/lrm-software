
#ifndef _ALN__H_
#define _ALN_H_

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "utils.h"
#include "fingerprint.h"
#include "index.h"
#include "hash.h"

typedef struct alignment
{
	u08 orient;
	u32 left, right;
	u32 score;
} Alignment;


struct ALN_Options
{
	u08 verbose;

	char * prefix;
	char * pac;
	char * si;
	char * spt;
	char * read;
	char * sam;
	char * dir;
	char * hash;

	u32 length;
	u32 interval;
	u32 band;
	u32 threshold;

	u32 items;
	u32 tsize;

	Fingerprint * pt;
	Hash * table;
};


#endif
