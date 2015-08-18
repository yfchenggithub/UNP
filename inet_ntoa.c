#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>


/*inet_addr: string ip to network bytes*/
/*inet_ntoa: network bytes to string ip*/

int main(int argc, char** argv)
{
	if (2 != argc)
	{
		printf("usage: %s string_ip\n", argv[0]);
		return -1;
	}
	
	in_addr_t u32_net_addr = inet_addr(argv[1]);
	if (INADDR_NONE == u32_net_addr)
	{
		printf("inet_addr fail\n");
		return -1;
	}
	
	struct in_addr addr;
	addr.s_addr = u32_net_addr;
	char* str_addr = inet_ntoa(addr);
	printf("str_addr %s\n", str_addr);
	return 0;
}
