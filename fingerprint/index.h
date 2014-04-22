
#ifndef _INDEX_H_
#define _INDEX_H_

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "types.h"
#include "utils.h"
#include "fingerprint.h"


struct SPT_Header
{
	u32 items, length, interval, band;
	u32 size;
	FType max[FPSize];
	FType min[FPSize];
};

struct Index_Options
{
	u08 verbose;
	u32 length, interval, band;
	u32 items;
	char * database;
	char * prefix;
	char * uspt;
	char * pac;
	char * si;
	char * dir;
};


#endif
