
#ifndef _ALN__H_
#define _ALN_H_

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "utils.h"
#include "fingerprint.h"

struct PT
{
	u32 pos;
	FType print[4];
};

struct ALN_Options
{
	u08 verbose;
	u32 length, interval, band;
	u32 items;
	u32 threshold;
	struct PT * pt;
	char * index;
	char * pac;
	char * spt;
	char * read;
	char * sam;
};


#endif
