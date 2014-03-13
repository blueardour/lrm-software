//  ******************************************************************//
//  author: chenp
//  version: 1.0
//	date: 2014-02-26
//  description: common functions
//  ******************************************************************//


#include "utils.h"

char * getFileName(const char * FilePath)
{
  char * filename;
  int i;
  filename = (char *)FilePath;
  for(i=strlen(FilePath)-1; i>=0; i--)
  {
    if(FilePath[i] == '\\' || FilePath[i] == '/')
		{
      filename = (char *)(FilePath + i + 1);
			break;
		}
  }
  return filename;
}

char * getFilePath(const char * FilePath)
{
  char * path;
  int i;
  path = (char *) malloc(strlen(FilePath));
  for(i=strlen(FilePath)-1; i>=0; i--)
  {
    if(FilePath[i] == '\\' || FilePath[i] == '/')
		{
			strncpy(path, FilePath, i+1);
			path[i+1] = 0;
			break;
		}
  }
	if(i == -1)
	{
		free(path);
		return NULL;
	}
	else return path;
}

char * getFileType(const char * FilePath)
{
  char * type;
  int i;
  type = (char *)FilePath;
  for(i=strlen(FilePath)-1; i>=0; i--)
  {
    if(FilePath[i] == '.' )
		{
			type = (char *)(FilePath + i + 1);
			break;
		}
  }
  return type;
}

// forward
// 0x01: copy to file
// 0x02: enable filter
int read2f_util(FILE *fp, char c, u08 forward, FILE * fp2, int filter)
{
	char s, ls;
	int n;

	if(forward & 0x01)
		if(fp2 == NULL) return -2;

	ls = 0; n = 1;
	while((s=fgetc(fp)) != EOF)
	{
		if(s==c)
		{
			return 0;
		}
		else if(s=='A' || s=='C' || s=='G' || s=='T' || s=='N')
		{
			if(forward & 0x01) fputc(s, fp2);
			if(forward & 0x02)
			{
				if(ls=='N' && s=='N') n++; else n = 1;
				if(n == filter) return -3;
				ls = s;
			}
		}
	}
	return -1;
}


