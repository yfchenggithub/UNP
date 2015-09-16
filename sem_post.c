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
	
	const char* _name = argv[1];
	sem_t* _newsem = sem_open(_name, 0);
	if (SEM_FAILED == _newsem)
	{
		printf("sem_open fail\n");
		return -1;
	}
	
	if (sem_post(_newsem) < 0)
	{
		printf("sem_wait fail\n");
		return -1;
	}	
	
	int sval;
	sem_getvalue(_newsem, &sval);
	printf("sem_getvalue %d\n", sval);
	return 0;
}
