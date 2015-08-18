#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>

int main(int argc, char** argv)
{
	if (2 != argc)
	{
		printf("usage:%s tos_value \n", argv[0]);
		return -1;
	}

	int fd = socket(AF_INET, SOCK_STREAM, 0);
	int old_tos = -1;
	socklen_t tos_len = sizeof(int);
	if (getsockopt(fd, IPPROTO_IP, IP_TOS, (void*)&old_tos, &tos_len) < 0)
	{
		printf("getsockopt fail\n");
		return -1;
	}
	printf("old_tos %d\n", old_tos);
	
	int new_tos = atoi(argv[1]);
	if (setsockopt(fd, IPPROTO_IP, IP_TOS, (const void*)&new_tos, sizeof(new_tos)) < 0)
	{
		printf("setsockopt fail\n");
		return -1;
	}	
	printf("setsockopt succ\n");	

	if (getsockopt(fd, IPPROTO_IP, IP_TOS, (void*)&old_tos, &tos_len) < 0)
	{
		printf("getsockopt fail\n");
		return -1;
	}
	
	printf("after set tos %d\n", old_tos);
	close(fd);
	return 0;
}
