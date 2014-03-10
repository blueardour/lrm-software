
#ifndef _UTILS_H_
#define _UTILS_H_

#include <string.h>
#include <stdio.h>

#define max2(a,b) ((a)>(b))?(a):(b)
#define max5(a,b,c,d,e) max2(max2(max2(a,b), max2(c,d)), e)

#define min2(a,b) ((a)>(b))?(b):(a)
#define min5(a,b,c,d,e) min2(min2(min2(a,b), min2(c,d)), e)

char * getFileName(char * FilePath);
char * getFileType(char * FilePath);
int read2b_util(FILE *fp, char c, int forward, char * buffer, int len);
int read2f_util(FILE *fp, char c, int forward, FILE * fp2);
int read2s_util(FILE *fp, char c, int forward);

unsigned int diff(unsigned int a, unsigned int b);

#endif
