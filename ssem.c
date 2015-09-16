#include <unistd.h>
#include <fcntl.h> 
#include <sys/stat.h>
#include <semaphore.h>
#include <stdio.h>

int main(int argc, char** argv)
{
	const char* _name = "sem.cap";
	sem_t* _newsem = sem_open(_name, O_CREAT);	
	if (SEM_FAILED == _newsem)
	{
		printf("sem_open fail\n");
		return -1;
	}
	else
	{
		printf("sem_open succ\n");
	}

	int val;
	int ret = sem_getvalue(_newsem, &val);
	printf("val %d\n", val);
	sem_wait(_newsem);
	sem_close(_newsem);
	return 0;
}
