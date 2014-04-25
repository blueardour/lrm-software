

#ifndef __HASH_
#define __HASH_

#include "fingerprint.h"
#include "types.h"

typedef struct Index_Key
{
	u32 i;
	u64 key;
} Index_Key;

typedef struct Index_Hash
{
	u32 left, right;
	u64 key;
} Index_Hash;

u64 getKey(FType * pt, int);
int compare_hash(const void * a, const void * b);
int judge_range(FType * pt);


#endif
