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
	sem_t* _newsem = sem_open(_name, flags, 0644, 1);
	if (SEM_FAILED == _newsem)
	{
		perror("sem_open fail");
		return -1;
	}
	
	int sval;
	if (sem_getvalue(_newsem, &sval) < 0)
	{
		printf("sem_getvalue fail\n");
	}	
	else
	{
		printf("sem_getvalue %d\n", sval);
	}
	return 0;
}
