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

/*从文件描述符fd读取相应的消息， recvfd 用以返回收取的描述字*/
ssize_t read_fd(int fd, void* ptr, size_t nbytes, int* recvfd)
{
	struct msghdr _msg;
	struct iovec _iov[1];
	ssize_t _nbytes;

	union
	{
		struct cmsghdr cm;
		char control[CMSG_SPACE(sizeof(int))];
	}control_un;
		
	_msg.msg_control = control_un.control;
	_msg.msg_controllen = sizeof(control_un.control);
	
	
	_msg.msg_name = NULL;
	_msg.msg_namelen = 0;
	_iov[0].iov_base = ptr;
	_iov[0].iov_len = nbytes;
	_msg.msg_iov = _iov;
	_msg.msg_iovlen = 1;
	if ((_nbytes = recvmsg(fd, &_msg, 0)) <= 0)
	{
		return _nbytes;
	}

	printf("recv %d _nbytes from %d\n", _nbytes, fd);
	struct cmsghdr* cmptr;
	if ((NULL != (cmptr = CMSG_FIRSTHDR(&_msg))) && (cmptr->cmsg_len == CMSG_LEN(sizeof(int))))
	{
		if (SOL_SOCKET != cmptr->cmsg_level)
		{
			printf("level != SOL_SOCKET\n");
		}
		
		if (SCM_RIGHTS != cmptr->cmsg_type)
		{
			printf("type != SCM_RIGHTS\n");
		}
		
		*recvfd = *((int*)CMSG_DATA(cmptr));
	}
	return _nbytes;
}

/*得到相应的文件描述字 权限之类的没有权限打开 没有许可证 所以多了一层*/
int my_open(const char* pathname, int mode)
{
	int fd;
	int sockfd[2];
	pid_t child_pid;
	int status;
	
	if (socketpair(AF_LOCAL, SOCK_STREAM, 0, sockfd) < 0)
	{
		printf("socketpair fail\n");
		return -1;
	}
	
	char arg_sockfd[10];
	char arg_mode[10];
	if (0 == (child_pid = fork()))
	{
		close(sockfd[0]);
		snprintf(arg_sockfd, sizeof(arg_sockfd), "%d", sockfd[1]);
		snprintf(arg_mode, sizeof(arg_mode), "%d", mode);
		execl("./openfile", "openfile", arg_sockfd, pathname, arg_mode, (char*)NULL);
		printf("child execl fail\n");
		exit(0);
	}
	
	close(sockfd[1]);
	waitpid(child_pid, &status, 0);
	char c;
	if (WIFEXITED(status))
	{
		printf("openfile exec succ\n");
		if ((status = WEXITSTATUS(status)) == 0)
		{
			read_fd(sockfd[0], &c, 1, &fd);
		}
		else
		{
			errno = status;
			fd = -1;
		}
	}
	close(sockfd[0]);
	return fd;
}

int main(int argc, char** argv)
{
	#define BUFFSIZE 1024
	char buf[BUFFSIZE];
	memset(buf, 0, sizeof(buf));
	
	if (2 != argc)
	{
		printf("usage:%s pathname\n", argv[0]);
		return -1;
	}
	
	int fd;
	const char* _pathname = argv[1];
	fd = my_open(_pathname, O_RDONLY);
	if (fd < 0)
	{
		printf("open %s fail\n", _pathname);
		return -1;
	}
	int nbytes;
	while ((nbytes = read(fd, buf, BUFFSIZE)) > 0)
	{
		write(STDOUT_FILENO, buf, strlen(buf));
		bzero(buf, BUFFSIZE);
	}
	return 0;
}
