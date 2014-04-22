

#include "hash.h"

// [17-22] 6 bits
u64 getKey(FType * pt)
{
    int i;
    u64 key;

    key = 0;
    for(i=0; i<FPSize; i++)
    {
        if(pt[i] > 0x003fffff) return -1;
        key = (key << 6) | ((pt[i] & 0x003f0000) >> 16);
    }

    return key;
}

int compare_hash(const void * a, const void * b)
{
	return ((Index_Hash *)a)->key - ((Index_Hash *)b)->key;
}

