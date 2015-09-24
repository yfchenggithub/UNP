#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*这个是客户程序；读取标准输入，发送到套接字，等待服务器的响应， 输出到标准输出*/
inline int max(int a, int b)
{
	return (a>b)?a:b;
}

char* gf_time(void)
{
	struct timeval tv;
	static char str[30];
	char* ptr;
	if (gettimeofday(&tv, NULL) < 0)
	{
		printf("gettimeofday fail\n");
	}
	
	ptr = ctime(&tv.tv_sec);
	strcpy(str, &ptr[11]);
	snprintf(str+8, sizeof(str)-8, ".%06ld", tv.tv_usec);

	return str;
}

/*对于fp sockfd 两个进行相应的监听*/
void str_cli(FILE* fp, int sockfd)
{
	#define MAXLINE 1024
	char to[MAXLINE];
	char fr[MAXLINE];
	memset(to, 0, MAXLINE);
	memset(fr, 0, MAXLINE);
	
	int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);	
	
	flags = fcntl(STDOUT_FILENO, F_GETFL, 0);
	fcntl(STDOUT_FILENO, F_SETFL, flags | O_NONBLOCK);

	flags = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
	
	char* toiptr;
	char* tooptr;
	char* friptr;
	char* froptr;
	toiptr = tooptr = to;
	friptr = froptr = fr;
	int stdineof = 0;
	
	int maxfd;
	maxfd = max(fileno(fp), sockfd) + 1;
	
	fd_set rset;
	fd_set wset;
	for (;;)
	{
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		/*标准输入的缓冲区有空间*/
		if ((0 == stdineof) && (toiptr < &to[MAXLINE]))
		{
			printf("%d to readset\n", STDIN_FILENO);
			FD_SET(STDIN_FILENO, &rset);
		}
		
		/*从套接口里面读取数据 因为缓冲区里面还有空间*/
		if (friptr < &fr[MAXLINE])
		{
			printf("%d to readset\n", sockfd);
			FD_SET(sockfd, &rset);
		}
		
		/*还有数据没有发送到套接字中, 因为标准输入的读取速度快于套接字的接收速度*/
		if (tooptr < toiptr)
		{
			printf("%d to writeset\n", sockfd);
			FD_SET(sockfd, &wset);
		}
		
		/*还有数据发送到标准输出中， 套接字的读取速度大于标准输出的接收速度*/
		if (froptr < friptr)
		{
			printf("%d to writeset\n", STDOUT_FILENO);
			FD_SET(STDOUT_FILENO, &wset);
		}
		
		/*一直阻塞到有描述符准备好相应的操作，读或写*/
		int ready_num = select(maxfd, &rset, &wset, NULL, NULL);
		
		if (ready_num < 0)
		{
			perror("select ");
			continue;
		}
		
		if (0 == ready_num)
		{
			printf("select timeout\n");
			continue;
		}
		
		int nread_bytes;
		if (FD_ISSET(STDIN_FILENO, &rset))	
		{
			printf("read on %d\n", STDIN_FILENO);
			int left_space = &to[MAXLINE] - toiptr;
			if ((nread_bytes = read(STDIN_FILENO, toiptr, left_space)) < 0)
			{
				if (EWOULDBLOCK != errno)
				{
					printf("select ready but read error\n");
				}
			}
			
			if (0 == nread_bytes)
			{
				fprintf(stderr, "%s: eof on %d\n", gf_time(), STDIN_FILENO);
				stdineof = 1;
				/*从标准输入读取的数据 已经全部发送给套掊字*/
				if (toiptr == tooptr)
				{
					shutdown(sockfd, SHUT_WR);
				}
			}
			else
			{
				/*正常读取流程*/
				fprintf(stderr, "%s: read %d bytes from stdin\n", gf_time(), nread_bytes);
				toiptr += nread_bytes;
				FD_SET(sockfd, &wset);
			}
		}
		
		if (FD_ISSET(sockfd, &rset))
		{
			printf("read on %d\n", sockfd);
			int read_space = &fr[MAXLINE] - friptr;
			if ((nread_bytes = read(sockfd, friptr, read_space)) < 0)
			{
				if (EWOULDBLOCK != errno)
				{
					printf("select ready but read error\n");
				}
				continue;
			}
			
			if (0 == nread_bytes)
			{
				fprintf(stderr, "%s: eof of sockfd\n", gf_time());
				if (stdineof)
				{
					return;
				}
				else
				{
					printf("server terminated\n");
				}
			}
			else
			{
				fprintf(stderr, "%s: read %d bytes from %d\n", gf_time(), nread_bytes, sockfd);
				friptr += nread_bytes;
				FD_SET(STDOUT_FILENO, &wset);
			}
		}

		int nwrite_bytes;
		int nwrite_space;
		if (FD_ISSET(STDOUT_FILENO, &wset) && ((nwrite_space = friptr - froptr) > 0))
		{
			printf("write on %d\n", STDOUT_FILENO);
			if ((nwrite_bytes = write(STDOUT_FILENO, froptr, nwrite_space)) < 0)
			{
				if (EWOULDBLOCK != errno)
				{
					printf("write error to stdout\n");
				}
			}
			else
			{
				fprintf(stderr, "%s: wrote %d bytes to stdout\n", gf_time(), nwrite_bytes);
				froptr += nwrite_bytes;
				if (friptr == froptr)
				{
					friptr = froptr = fr;
				}
			}
		}


		if (FD_ISSET(sockfd, &wset) && ((toiptr - tooptr) > 0))
		{
			printf("write on %d\n", sockfd);
			nwrite_space = toiptr - tooptr;
			if ((nwrite_bytes = write(sockfd, tooptr, nwrite_space)) < 0)
			{
				if (EWOULDBLOCK != errno)
				{
					printf("write to sockfd fail\n");
				}
			}
			else
			{
				fprintf(stderr, "%s: write %d bytes to socket\n", gf_time(), nwrite_bytes);
				tooptr += nwrite_bytes;
				if (tooptr == toiptr)
				{
					toiptr = tooptr = to;
					if (stdineof)
					{
						shutdown(sockfd, SHUT_WR);
					}
				}
			}
		}
	}
}


int main(int argc, char** argv)
{
	str_cli(stdin, 2);
	return 0;
}
