
#include <stdio.h>
#include <unistd.h>

int main()
{
	unsigned int a, b;
	char c;
	pid_t pid;
	char buffer[100];

	int array[100];

  //system("wget ftp://219.219.218.142/Software/PXE/PXE/WinPE-MaoTao-Single-1111.iso");
  printf("Download OK, sleeping 10s \r\n");
  //sleep(10);
	printf("Sizoef(short):%ld \r\n",sizeof(short));

	a = b = 1000;
	a= a * b;
	c = 150;
	printf("c:%d\r\n", c);
	printf("Sizoef(short):%d \r\n",a);

	pid = getpid();
	sprintf(buffer, "/proc/%d/status", pid);
	printf("Pid:%d s:%s\r\n",pid, buffer);

	a = 0;
	if(a++, a==1) printf("A");
	else printf("B");

	printf("Sizoef(array):%d %d \r\n", sizeof(array), sizeof(long));
  return 0;
}
