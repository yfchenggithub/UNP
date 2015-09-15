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

int main(int argc, char** argv)
{
	if (3 != argc)
	{
		printf("%s host service\n", argv[0]);
		return -1;
	}
	
	const char* _host = argv[1];
	const char* _service = argv[2];
	struct addrinfo* _addrinfo = host_serv(_host, _service, AF_INET, SOCK_STREAM);
	if (NULL == _addrinfo)
	{
		printf("host_serv fail\n");
		return -1;
	}
	dump_addr_info(_addrinfo);
	freeaddrinfo(_addrinfo);
	return 0;
}
