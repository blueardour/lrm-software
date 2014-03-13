
#ifndef _FINGERPRINT_H_
#define _FINGERPRINT_H_

#include "types.h"

struct chromosome
{
  u32 slen;
  u32 nb, ne;
	int sections;
	int nlen;
	char * sn;
};

struct Reference
{
  int seqs;
	u32 A, C, T, G, N;
  struct chromosome * chrom;
};

struct position
{
	u32 pos;
};

struct fingerprint
{
	struct position pos;
	FType print[4];
};

#endif
