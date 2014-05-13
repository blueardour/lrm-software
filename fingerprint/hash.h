

#ifndef __HASH_
#define __HASH_

#include "fingerprint.h"
#include "types.h"

typedef struct Sort
{
	u32 i;
	u32 pos;
	u64 key;
} Sort;

typedef struct Hash
{
	u32 beg, end;
	u64 key;
} Hash;

u64 get_key(FType * pt);
int compare_hash(const void * a, const void * b);
int judge_range(FType * pt);


#endif
