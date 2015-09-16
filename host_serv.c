#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <net/if.h>
#include <sys/ioctl.h>

struct addrinfo* host_serv(const char* host, const char* service, int family, int sock_type)
{
	struct addrinfo hint;
	struct addrinfo* res;
	bzero(&hint, sizeof(struct addrinfo));
	
	hint.ai_flags = AI_CANONNAME;
	hint.ai_family = family;
	hint.ai_socktype = sock_type;

	if (0 == getaddrinfo(host, service, &hint, &res))
	{
		printf("getaddrinfo succ\n");
		return res;
	}
	return NULL;
}

void dump_addr_info(struct addrinfo* _addr)
{
	struct addrinfo* tmp_addr = _addr;
	while (tmp_addr)
	{
		printf("_addr->ai_canonname %s\n", tmp_addr->ai_canonname);
		printf("_addr->ai_addrlen %d\n", tmp_addr->ai_addrlen);
		char buf[INET_ADDRSTRLEN];
		bzero(buf, sizeof(buf));
		printf("_addr->ai_addr %s\n", inet_ntop(AF_INET, &((struct sockaddr_in*)(tmp_addr->ai_addr))->sin_addr,buf, INET_ADDRSTRLEN));
		tmp_addr = tmp_addr->ai_next;
	}
}


int tcp_connect(const char* _host, const char* _service)
{
	struct addrinfo hints;
	hints.ai_family = AF_INET;
	hints.ai_flags = 0;
	hints.ai_socktype = SOCK_STREAM;
	
	struct addrinfo* result;
	if (0 == getaddrinfo(_host, _service, &hints, &result))
	{
		printf("getaddrinfo succ\n");
		struct addrinfo* tmp_addr = result;
		while (tmp_addr)
		{
			int sockfd = socket(AF_INET, SOCK_STREAM, 0);
			if (sockfd < 0)
			{
				printf("socket fail\n");
				continue;
			}
			
			socklen_t _sock_len = sizeof(struct sockaddr);
		
			char buf[INET_ADDRSTRLEN];
			bzero(buf, INET_ADDRSTRLEN);
			struct sockaddr_in* _addr = (struct sockaddr_in*)(result->ai_addr);
			if (connect(sockfd, result->ai_addr, _sock_len) < 0)
			{
				printf("connect [%s:%d]fail\n", inet_ntop(AF_INET, &_addr->sin_addr, buf, INET_ADDRSTRLEN), ntohs(_addr->sin_port));
				close(sockfd);
			}
			else
			{
				printf("connect [%s:%d]succ\n", inet_ntop(AF_INET, &_addr->sin_addr, buf, INET_ADDRSTRLEN), ntohs(_addr->sin_port));
				break;	
			}
			tmp_addr = tmp_addr->ai_next;
		}
	}
	freeaddrinfo(result);
	return -1;
}

int tcp_listen(const char* _host, const char* _service, socklen_t* _addrlen)
{
	struct addrinfo* _result;
	struct addrinfo hints;
	hints.ai_flags = 0;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	
	if (0 == getaddrinfo(_host, _service, &hints, &_result))
	{
		printf("getaddrinfo succ\n");
		struct sockaddr_in* _tmp_sockaddr = (struct sockaddr_in*)(_result->ai_addr);
		struct addrinfo* _tmp_addr = _result;
		while (_tmp_addr)
		{
			int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
			if (sock_fd < 0)
			{
				printf("socket fail\n");
				continue;
			}
			
			struct sockaddr_in serve_addr;
			serve_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
			serve_addr.sin_port = htons(5555);
			serve_addr.sin_family = AF_INET;

			if (bind(sock_fd, (struct sockaddr*)&serve_addr, sizeof(struct sockaddr)) < 0)
			{
				printf("bind fail\n");
				close(sock_fd);
				continue;
			}
			
			if (0 == listen(sock_fd, 5))
			{
				printf("listen succ\n");
				break;
			}
			_tmp_addr = _tmp_addr->ai_next;
		}
	}	
	freeaddrinfo(_result);
}

void my_getnameinfo()
{
	struct sockaddr_in _addr;
	_addr.sin_family = AF_INET;
	_addr.sin_addr.s_addr = inet_addr("180.97.33.108");
	_addr.sin_port = htons(80);
	socklen_t _addr_len = sizeof(struct sockaddr_in);

	char _host[NI_MAXHOST];
	char _service[NI_MAXSERV];
	bzero(_host, NI_MAXHOST);
	bzero(_service, NI_MAXSERV);
	if (0 == getnameinfo((struct sockaddr*)&_addr, _addr_len, _host, NI_MAXHOST, _service, NI_MAXSERV,0))
	{
		printf("getnameinfo succ\n");
		printf("_host %s\n", _host);
		printf("_service %s\n", _service);
	}
	else
	{
		perror("getnameinfo fail");
	}
}

int main(int argc, char** argv)
{
	my_getnameinfo();
	return 0;
}
