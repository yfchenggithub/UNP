#include <unistd.h>
#include <fcntl.h> 
#include <sys/stat.h>
#include <semaphore.h>
#include <stdio.h>

int main(int argc, char** argv)
{
	if (2 != argc)
	{
		printf("%s name\n", argv[0]);
		return -1;
	}
	
	int flags = O_CREAT | O_RDWR;
	const char* _name = argv[1];
	sem_unlink(_name);
	return 0;
}
