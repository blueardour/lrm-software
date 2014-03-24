
#ifndef _FINGERPRINT_H_
#define _FINGERPRINT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"

#define FPSize 12

typedef struct Fingerprint12 Fingerprint;

extern u08 nst_nt4_table[256];

inline FType value(FType a, FType b);
FType estimate(FType *, FType *, int );

void stampFinger4(FType *, char *, u32);
void stampFinger8(FType *, char *, u32);
void stampFinger12(FType *, char *, u32);
void stampFinger(FType *, char *, u32);

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

struct Fingerprint4
{
	u32 pos;
	FType print[4];
};

struct Fingerprint8
{
	u32 pos;
	FType print[8];
};

struct Fingerprint10
{
	u32 pos;
	FType print[10];
};

struct Fingerprint12
{
	u32 pos;
	FType print[12];
};


#endif

