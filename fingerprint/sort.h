
#ifndef _FINGERPRINT_SORT_H_
#define _FINGERPRINT_SORT_H_

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "types.h"
#include "utils.h"
#include "index.h"
#include "hash.h"

struct Sort_Options
{
	u08 verbose;
	char * prefix;
	char * pattern;
	char * uspt;
	char * spt;
	char * info;
	char * hash;
};

#endif

