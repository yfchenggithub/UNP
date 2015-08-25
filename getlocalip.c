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
	int sockfd;
	socklen_t len;
	struct sockaddr_in cliaddr;
	struct sockaddr_in servaddr;
	
	if (3 != argc)
	{
		printf("usage: %s server_ip server_port\n", argv[0]);
		return -1;
	}
	
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	bzero(&servaddr, sizeof(servaddr));	
	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, argv[1], (struct in_addr*)&(servaddr.sin_addr));
	servaddr.sin_port = htons(atoi(argv[2]));
	
	if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
	{
		printf("connect fail\n");
		return -1;
	}
	
	len = sizeof(cliaddr);
	getsockname(sockfd, (struct sockaddr*)&cliaddr, &len);
	
	char bind_ip[INET_ADDRSTRLEN];
	memset(bind_ip, 0, sizeof(bind_ip));
	
	if (NULL == inet_ntop(AF_INET, (struct sockaddr*)&cliaddr.sin_addr, bind_ip, INET_ADDRSTRLEN))
	{
		printf("inet_ntop fail\n");
		return -1;
	}
	printf("bind_ip %s bind_port %d\n", bind_ip, ntohs(cliaddr.sin_port));
	return 0;
}
