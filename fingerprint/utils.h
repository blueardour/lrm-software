
#ifndef _UTILS_H_
#define _UTILS_H_

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "fingerprint.h"

#define max2(a,b) ((a)>(b))?(a):(b)
#define max5(a,b,c,d,e) max2(max2(max2(a,b), max2(c,d)), e)

#define min2(a,b) ((a)>(b))?(b):(a)
#define min5(a,b,c,d,e) min2(min2(min2(a,b), min2(c,d)), e)


char * getFileName(const char * FilePath);
char * getFileType(const char * FilePath);
char * getFilePath(const char * FilePath);

int read2f_util(FILE *, char, u08, FILE *, int);
int read2b_util(FILE *, char, u08, char *, int);
u32 newRand(u32 range, int seed);

#endif
