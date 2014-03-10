//  ******************************************************************//
//  author: chenp
//  version: 1.0
//	date: 2014-02-26
//  description: common functions
//  ******************************************************************//


#include "utils.h"


char * getFileName(char * FilePath)
{
  char * filename;
  int i;
  filename = FilePath;
  for(i=0; i<strlen(FilePath); i++)
  {
    if(FilePath[i] == '\\' || FilePath[i] == '/')
      filename = FilePath + i + 1;  
  }
  return filename;
}

char * getFileType(char * FilePath)
{
  char * type;
  int i;
  type = FilePath;
  for(i=strlen(FilePath)-1; i>=0; i--)
  {
    if(FilePath[i] == '.' )
	{
		type = FilePath + i + 1;
		break;
	}
  }
  return type;
}

int read2b_util(FILE *fp, char c, int forward, char * buffer, int len)
{
	char s;
	int i;
	i=0;
	while((s=fgetc(fp)) != EOF)
	{
		if(s==c) return 0;
		else if(forward != 0 && len != 0) 
			if(s=='A' || s=='C' || s=='G' || s=='T') 
			{
				buffer[i++] = s;
				--len;
			}
	}
	if(forward != 0 && len == 0) return -1;
	else return -2;
}

int read2f_util(FILE *fp, char c, int forward, FILE * fp2)
{
	char s;
	while((s=fgetc(fp)) != EOF)
	{
		if(s==c)
		{
			if(fp2 != NULL) fputc(s, fp2);
			return 0;
		}
		else if(forward == 1)
		{
			if(s=='A' || s=='C' || s=='G' || s=='T') if(fp2 != NULL) fputc(s, fp2);
		}
		else if(fp2 != NULL) fputc(s, fp2);
	}
	return -2;
}

int read2s_util(FILE *fp, char c, int forward)
{
	char s;
	while((s=fgetc(fp)) != EOF)
	{
		if(s==c)
		{
			putchar(s);
			return 0;
		}
		else if(forward == 1)
		{
			if(s=='A' || s=='C' || s=='G' || s=='T') putchar(s);
		}
		else putchar(s);
	}
	return -2;
}

unsigned int diff(unsigned int a, unsigned int b)
{
	if(a>b) return a - b;
	else return b - a;
}


