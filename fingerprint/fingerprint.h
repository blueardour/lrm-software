
#ifndef _FINGERPRINT_H_
#define _FINGERPRINT_H_

#include "types.h"

struct piece
{
	u32 plen;
	u32 nb;
};

struct chromosome
{
  u32 slen;
	int pnum;
	int nlen;
	char * sn;
	struct piece * pie;
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
