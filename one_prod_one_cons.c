#include <unistd.h>
#include <fcntl.h> 
#include <sys/stat.h>
#include <semaphore.h>
#include <stdio.h>

#define NBUFF 10
#define SEM_MUTEX "mutex"
#define SEM_NUM_EMPTY "num_empty"
#define SEM_NUM_STORE "num_store"

int num_items;

struct
{
	int m_buf[NBUFF];
	sem_t* m_mutex;
	sem_t* m_num_empty;
	sem_t* m_num_store;	
}shared;

void* produce(void* arg)
{
	int i;
	for (i=0; i<num_items; ++i)
	{
		sem_wait(shared.m_num_empty);
		sem_wait(shared.m_mutex);
		shared.m_buf[i] = i;
		printf("shared.m_buf[%d] %d\n", i, shared.m_buf[i]);
		sem_post(shared.m_mutex);
		sem_post(shared.m_num_store);
	}
	return NULL;
}

void* comsume(void* arg)
{
	int i;
	for (i=0; i<num_items; ++i)
	{
		sem_wait(shared.m_num_store);
		sem_wait(shared.m_mutex);
		if (shared.m_buf[i] != i)
		{
			printf("shared.m_buf[%d] %d\n", shared.m_buf[i], i);
		}
		sem_post(shared.m_mutex);
		sem_post(shared.m_num_empty);
	}	
	return NULL;
}

int main(int argc, char** argv)
{
	if (2 != argc)
	{
		printf("%s items\n", argv[0]);
		return -1;
	}
	
	num_items = atoi(argv[1]);
	int flags = O_CREAT | O_RDWR;
	shared.m_mutex = sem_open(SEM_MUTEX, flags, 0644, 1);
	shared.m_num_empty = sem_open(SEM_NUM_EMPTY, flags, 0644, NBUFF);
	shared.m_num_store = sem_open(SEM_NUM_STORE, flags, 0644, 0);
	
	int sval;
	sem_getvalue(shared.m_num_store, &sval);
	printf("m_num_store %d\n", sval);
	
	sem_getvalue(shared.m_num_empty, &sval);
	printf("m_num_empty %d\n", sval);

	sem_getvalue(shared.m_mutex, &sval);
	printf("m_mutex %d\n", sval);

	pthread_t tid_produce;
	pthread_create(&tid_produce, NULL, produce, NULL);
	pthread_t tid_consumer;	
	pthread_create(&tid_consumer, NULL, comsume, NULL);

	pthread_join(tid_produce, NULL);
	pthread_join(tid_consumer, NULL);

	sem_unlink(SEM_MUTEX);
	sem_unlink(SEM_NUM_EMPTY);
	sem_unlink(SEM_NUM_STORE);
	return 0;
}
