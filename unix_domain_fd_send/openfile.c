#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

ssize_t write_fd(int fd, void* ptr, size_t nbytes, int sendfd)
{
	struct msghdr msg;
	struct iovec iov[1];
	
	union
	{
		struct cmsghdr  cm;
		char control[CMSG_SPACE(sizeof(int))];
	}control_un;
	
	msg.msg_control = control_un.control;
	msg.msg_controllen = sizeof(control_un.control);

	struct cmsghdr* cmptr;
	cmptr = CMSG_FIRSTHDR(&msg);
	cmptr->cmsg_len = CMSG_LEN(sizeof(int));
	cmptr->cmsg_level = SOL_SOCKET;
	cmptr->cmsg_type = SCM_RIGHTS;
	*((int*)CMSG_DATA(cmptr)) = sendfd;

	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	iov[0].iov_base = ptr;
	iov[0].iov_len = nbytes;
	
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	return (sendmsg(fd, &msg, 0));	
}

int main(int argc, char** argv)
{
	int fd;
	if (4 != argc)
	{
		printf("usage: %s sockfd filename mode\n", argv[0]);
		return -1;
	}	
	
	printf("enter %s\n", argv[0]);	
	const int _sockfd = atoi(argv[1]);
	const char* _filename = argv[2];
	const int _mode = atoi(argv[3]);
	if ((fd = open(_filename, _mode)) < 0)
	{
		exit((errno>0)?errno:255);
	}
	
	int nbytes;	
	if ((nbytes = write_fd(_sockfd, "", 1, fd)) < 0)
	{
		exit((errno>0)?errno:255);
	}
	
	printf("send %d %d bytes\n", _sockfd, nbytes);
	return 0;
}
