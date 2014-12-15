

#include "hash.h"

// [17-22] 6 bits
u64 get_key(FType * pt)
{
    int i;
    u64 key;

    for(i=0,key=0; i<FPSize; i++) key = (key << 6) | ((pt[i] & 0x003f0000) >> 16);
    return key;
}

int judge_range(FType * pt)
{
	int i;
	for(i=0; i<FPSize; i++) if(pt[i] > 0x003fffff) return -1;
	return 0;
}

int compare_hash(const void * a, const void * b)
{
	return ((Hash *)a)->key - ((Hash *)b)->key;
}

