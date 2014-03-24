
#ifndef _ALN__H_
#define _ALN_H_

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "utils.h"
#include "fingerprint.h"

struct ALN_Options
{
	u08 verbose;
	u32 length, interval, band;
	u32 items;
	u32 threshold;
	Fingerprint * pt;
	char pattern[4];
	char * prefix;
	char * pac;
	char * si;
	char * spt;
	char * read;
	char * sam;
};


#endif
