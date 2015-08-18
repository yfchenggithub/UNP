#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

int main(int argc, char** argv)
{
	if (2 != argc)
	{
		printf("usage:%s recv_buf_size \n", argv[0]);
		return -1;
	}

	int fd = socket(AF_INET, SOCK_STREAM, 0);
	int recv_buf_size = atoi(argv[1]);
		
	if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)&recv_buf_size, sizeof(recv_buf_size)) < 0)
	{
		printf("setsockopt fail\n");
		return -1;
	}
	
	int buf_size = 0;
	socklen_t buf_size_len = sizeof(buf_size);
	if (getsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)&buf_size, &buf_size_len) < 0)
	{
		printf("getsockopt fail\n");
		return -1;
	}
	printf("buf_size %d\n", buf_size);
	return 0;
}
