#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void str_cli(FILE* fp, int fd)
{
	fd_set read_set;
	int maxfd;
	FD_ZERO(&read_set);
	int tmpfd = fileno(fp);
	#define MAXSIZE 1024
	char buf[MAXSIZE];
	bzero(buf, MAXSIZE);
	int nbytes;
	for (;;)		
	{
		maxfd = fd + 1;
		FD_ZERO(&read_set);
		FD_SET(fd, &read_set);
		FD_SET(tmpfd, &read_set);
		int nfd_ready = select(maxfd, &read_set, NULL, NULL, NULL);
		if (FD_ISSET(fd, &read_set))
		{
			if ((nbytes = read(fd, buf, MAXSIZE)) > 0)
			{
				write(fileno(stdout), buf, strlen(buf));
			}
			
			if (0 == nbytes)
			{
				printf("server terminated\n");
			}
		}

		if (FD_ISSET(tmpfd, &read_set))
		{
			if ((nbytes = read(tmpfd, buf, MAXSIZE)) > 0)
			{
				write(fd, buf, strlen(buf));
			}			
			
			if (0 == nbytes)
			{
				shutdown(fd, SHUT_WR);
				FD_CLR(tmpfd, &read_set);
				continue;
			}
		}
	}
}

int main(int argc, char** argv)
{
	if (2 != argc)
	{
		printf("usage: %s pathname\n", argv[0]);
		return -1;
	}
	
	int sockfd = socket(AF_LOCAL, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		printf("socket fail\n");
		return -1;
	}

	struct sockaddr_un server;
	server.sun_family = AF_LOCAL;
	const char* _pathname = argv[1];
	strncpy(server.sun_path, _pathname, strlen(_pathname));
	if (connect(sockfd, (struct sockaddr*)&server, sizeof(server)) < 0)
	{
		printf("connect server fail\n");
		return -1;
	}	
	str_cli(stdin, sockfd);
	return 0;	
}
