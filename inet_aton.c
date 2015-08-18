#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>


/*
192.168.28.154  --> 0x9a1ca8c0 inet_aton: 字符串到网络字节序
*/
int main(int argc, char** argv)
{
	if (2 != argc)
	{
		printf("usage:%s str_ip\n", argv[0]);
		return -1;
	}
	
	struct sockaddr_in addr;
	/*non-zero if the address is valid*/
	if (inet_aton(argv[1], &addr.sin_addr))
	{
		printf("inet_aton succ\n");
	}	
	printf("0x%x \n", addr.sin_addr.s_addr);

	return 0;
}
