
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char ** argv)
{
  int i;
  if(argc != 3) { printf("Error, input parameter number should not be %d\r\n",argc); exit(0); }

  for(i=0; i<strlen(argv[1]); i++)
  {
    if(argv[1][i] == argv[2][i]) continue;
    else { while(argv[1][i] != '/') i--; break;}
  }
  argc = i;
  while(i<strlen(argv[2]))
  {
    if(argv[2][i] == '/') printf("../");
    i++;
  }
  printf("%s", argv[1]+argc+1);
  return 0;
}
