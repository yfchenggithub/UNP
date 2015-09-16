#include <unistd.h>
#include <fcntl.h> 
#include <sys/stat.h>
#include <semaphore.h>
#include <stdio.h>

int main(int argc, char** argv)
{
	if (3 != argc)
	{
		printf("%s sem_init_num name\n", argv[0]);
		return -1;
	}
	const char* _name = argv[2];
	sem_t* _newsem = sem_open(_name, O_CREAT|O_EXCL, 0644, atoi(argv[1]));	
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
	ret = sem_getvalue(_newsem, &val);
	printf("val %d\n", val);
	sem_wait(_newsem);
	ret = sem_getvalue(_newsem, &val);
	printf("val %d\n", val);
	sem_post(_newsem);
	ret = sem_getvalue(_newsem, &val);
	printf("val %d\n", val);
	sem_close(_newsem);
	return 0;
}
