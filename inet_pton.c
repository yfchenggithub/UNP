#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>

typedef enum inet_pton_ret
{
	RET_SUCC = 1,
	RET_INVALID_ADDR = 0,
	RET_AF_NOT_SUPPORT = -1,
}inet_pton_ret;

int main(int argc, char** argv)
{
	if (2 != argc)
	{
		printf("usage: %s string_ip\n", argv[0]);
		return -1;
	}
	
	struct in_addr addr;
	if (RET_SUCC == inet_pton(AF_INET, argv[1], (void*)&addr))
	{
		printf("inet_pton succ\n");
	}
	
	printf("0x%x\n", addr.s_addr);
	return 0;
}
