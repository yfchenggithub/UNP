#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>


int main(int argc, char** argv)
{
	if (2 != argc)
	{
		printf("usage: %s string_ip\n", argv[0]);
		return -1;
	}
	
	struct in_addr addr;
	addr.s_addr = inet_addr(argv[1]);
	
	char addr_buf[INET_ADDRSTRLEN];
	memset(addr_buf, 0, sizeof(addr_buf));
	if (NULL != inet_ntop(AF_INET, (void*)&addr, addr_buf, INET_ADDRSTRLEN))
	{
		printf("inet_ntop succ\n");
	}
	
	printf("addr: %s\n", addr_buf);
	return 0;
}
