#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <net/if.h>
#include <sys/ioctl.h>

int main(int argc, char** argv)
{
	if (3 != argc)
	{
		printf("%s host service\n", argv[0]);
		return -1;
	}
	
	const char* host = argv[1];
	const char* service = argv[2];
	
	struct hostent* phost_ent;
	if (NULL == (phost_ent = gethostbyname(host)))
	{
		printf("gethostbyname fail\n");
		return -1;
	}
	
	struct servent* pserv_ent;
	if (NULL == (pserv_ent = getservbyname(service, "tcp")))
	{
		printf("getservbyname fail\n");
		return -1;
	}
	
	int fd;
	char** ppaddr = phost_ent->h_addr_list;
	for (; NULL!=*ppaddr; ++ppaddr)
	{
		fd = socket(AF_INET, SOCK_STREAM, 0);
		if (fd < 0)
		{
			printf("socket fail\n");
			return -1;
		}
		struct sockaddr_in serv_addr;
		serv_addr.sin_family = AF_INET;
		memcpy(&serv_addr.sin_addr, *ppaddr, sizeof(struct in_addr));
		serv_addr.sin_port = pserv_ent->s_port;
		socklen_t len = sizeof(struct sockaddr_in);
		if (0 == connect(fd, (struct sockaddr*)&serv_addr, len))
		{
			printf("connect succ\n");
			break;
		}
		close(fd);	
	}
	
	#define MAX_SIZE 1024
	char buf[MAX_SIZE];
	int nbytes;
	while ((nbytes = read(fd, buf, MAX_SIZE)) > 0)
	{
		write(1, buf, nbytes);
	}
	return 0;
}












