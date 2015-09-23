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

void sig_chld(int sig)
{
	printf("recv %s\n", strsignal(sig));
	return;
}

void str_echo(int fd)
{
	ssize_t nbytes;
	#define MAXSIZE 1024
	char buf[MAXSIZE];
	memset(buf, 0, sizeof(buf));	
	
again:
	while ((nbytes = read(fd, buf, MAXSIZE)) > 0)
	{
		write(fd, buf, strlen(buf));
	}
	
	if (( nbytes < 0) && (EINTR == errno))
	{
		goto again;
	}
	
	if (nbytes < 0)
	{
		printf("read fail\n");
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
	
	const char* _pathname = argv[1];
	if (unlink(_pathname) < 0)
	{
		printf("unlink %s fail, that is ok ignore\n", _pathname);
	}	
	
	struct sockaddr_un addr;
	addr.sun_family = AF_LOCAL;
	strncpy(addr.sun_path, argv[1], sizeof(addr.sun_path) - 1);
	int struct_len = SUN_LEN(&addr);
	if (bind(sockfd, (const struct sockaddr*)&addr, struct_len) < 0)
	{
		printf("bind fail\n");
		return -1;
	}
	
	#define LISTEN_QUEUE_NUM 10
	if (listen(sockfd, LISTEN_QUEUE_NUM) < 0)
	{
		printf("listen fail\n");
		return -1;
	}	

	struct sigaction new_action;
	new_action.sa_flags = 0;
	new_action.sa_handler = sig_chld;
	sigemptyset(&new_action.sa_mask);	
	
	if (sigaction(SIGCHLD, &new_action, NULL) < 0)
	{
		printf("install handler for SIGCHLD fail\n");
		return -1;
	}

	struct sockaddr_un cliaddr;
	for (;;)
	{
		int clilen = sizeof(cliaddr);
		int conn_fd = accept(sockfd, (struct sockaddr*)&cliaddr, &clilen);
		pid_t child_pid;
		if (0 == (child_pid = fork()))
		{
			close(sockfd);
			str_echo(conn_fd);
			exit(0);
		}
		close(conn_fd);
	}		
	return 0;
}
