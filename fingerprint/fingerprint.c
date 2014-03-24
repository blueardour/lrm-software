

#include "fingerprint.h"

u08 nst_nt4_table[256] = {
		4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
		4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
		4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 5 /*'-'*/, 4, 4,
		4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
		4, 0, 4, 1,  4, 4, 4, 2,  4, 4, 4, 4,  4, 4, 4, 4, 
		4, 4, 4, 4,  3, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
		4, 0, 4, 1,  4, 4, 4, 2,  4, 4, 4, 4,  4, 4, 4, 4, 
		4, 4, 4, 4,  3, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
		4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
		4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
		4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
		4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
		4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
		4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
		4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
		4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4
};

inline FType value(FType a, FType b)
{
	  if(a > b) return a - b; else return b - a;
}

FType estimate(FType * pt1, FType * pt2, int size)
{
  int i;
  FType result;

	result = 0;
	 for(i=0; i<size; i++)
	 {
		 result += value(pt1[i], pt2[i]);
	 }
	 return result;
}

// calculate 4 forward fingerprint
void stampFinger4(FType * print, char * buffer, u32 len)
{
	static FType finger[4];
	static u32 i;

	print[0] = print[1] = print[2] = print[3] = 0;
	finger[0] = finger[1] = finger[2] = finger[3] = 0;

	for(i=0; i<len; i++)
	{
		switch(buffer[i])
		{
			case 'A': finger[0]++; break;
			case 'C': finger[1]++; break;
			case 'G': finger[2]++; break;
			case 'T': finger[3]++; break;
		}

		print[0] += finger[0];
		print[1] += finger[1];
		print[2] += finger[2];
		print[3] += finger[3];
	}
}

// calculate 4 forward fingerprint and 4 backward fingerprint
void stampFinger8(FType * print, char * buffer, u32 len)
{
	static FType finger[8];
	static u32 i;

	print[0] = print[1] = print[2] = print[3] = 0;
	print[4] = print[5] = print[6] = print[7] = 0;
	finger[0] = finger[1] = finger[2] = finger[3] = 0;
	finger[4] = finger[5] = finger[6] = finger[7] = len;

	for(i=0; i<len; i++)
	{
		print[4] += finger[4];
		print[5] += finger[5];
		print[6] += finger[6];
		print[7] += finger[7];
		
		switch(buffer[i])
		{
			case 'A': finger[0]++; finger[4]--; break;
			case 'C': finger[1]++; finger[5]--; break;
			case 'G': finger[2]++; finger[6]--; break;
			case 'T': finger[3]++; finger[7]--; break;
		}
		
		print[0] += finger[0];
		print[1] += finger[1];
		print[2] += finger[2];
		print[3] += finger[3];
	}
	
	print[4] -= finger[4] * len;
	print[5] -= finger[5] * len;
	print[6] -= finger[6] * len;
	print[7] -= finger[7] * len;
}


// calculate 4 forward fingerprint and 4 backward fingerprint
// and addition 
void stampFinger(FType * print, char * buffer, u32 len)
{
	static FType stamp[12];
	static int i;

	print[0] = 0; print[1] = 0;	print[2] = 0;	print[3] = 0;
	print[4] = 0;	print[5] = 0;	print[6] = 0;	print[7] = 0;
	print[8] = 0;	print[9] = 0;	print[10] = 0;print[11] = 0;
	stamp[0] = 0;	stamp[1] = 0;	stamp[2] = 0;	stamp[3] = 0;
	stamp[4] = 0;	stamp[5] = 0;	stamp[6] = 0;	stamp[7] = 0;
	stamp[8] = 0;	stamp[9] = 0;	stamp[10] = 0;stamp[11] = 0;

	for(i=0; i<=len/2; i++)
	{
		switch(buffer[i])
		{
			case 'A': stamp[0]++; break;
			case 'C': stamp[1]++; break;
			case 'G': stamp[2]++; break;
			case 'T': stamp[3]++; break;
		}

		print[0] += stamp[0];
		print[1] += stamp[1];
		print[2] += stamp[2];
		print[3] += stamp[3];
	}
	
	for(; i<len; i++)
	{
		switch(buffer[i])
		{
			case 'A': stamp[0]++; stamp[4]++; break;
			case 'C': stamp[1]++; stamp[5]++; break;
			case 'G': stamp[2]++; stamp[6]++; break;
			case 'T': stamp[3]++; stamp[7]++; break;
		}

		print[0] += stamp[0];
		print[1] += stamp[1];
		print[2] += stamp[2];
		print[3] += stamp[3];
		print[4] += stamp[4];
		print[5] += stamp[5];
		print[6] += stamp[6];
		print[7] += stamp[7];
	}
	
	for(i=len-1; i>len/2; i--)
	{
		switch(buffer[i])
		{
			case 'A': stamp[8]++; break;
			case 'C': stamp[9]++; break;
			case 'G': stamp[10]++; break;
			case 'T': stamp[11]++; break;
		}

		print[8]  += stamp[8];
		print[9]  += stamp[9];
		print[10] += stamp[10];
		print[11] += stamp[11];
	}
	
	for(; i>=0; i--)
	{
		switch(buffer[i])
		{
			case 'A': stamp[4]++; stamp[8]++; break;
			case 'C': stamp[5]++; stamp[9]++; break;
			case 'G': stamp[6]++; stamp[10]++; break;
			case 'T': stamp[7]++; stamp[11]++; break;
		}

		print[4] += stamp[4];
		print[5] += stamp[5];
		print[6] += stamp[6];
		print[7] += stamp[7];
		print[8] += stamp[8];
		print[9] += stamp[9];
		print[10] += stamp[10];
		print[11] += stamp[11];
	}
}

