#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <pthread.h>



void log_info(const char* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
}

int main(int argc, char** argv)
{
	if (3 != argc)
	{
		log_info("usage:%s ip port\n", argv[0]);
		return -1;
	}
	
	const char* _ip = argv[1];
	const int _port = atoi(argv[2]);
	
	struct sockaddr_in _server;
	_server.sin_family = AF_INET;
	_server.sin_port = htons(_port);
	inet_pton(AF_INET, _ip, &_server.sin_addr);
	int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	socklen_t addr_len = sizeof(_server);
	bind(listen_fd, (struct sockaddr*)&_server, addr_len);
	listen(listen_fd, 5);
	
	int conn_fd;

	if (conn_fd = accept(listen_fd, NULL, NULL))
	{
		close(STDOUT_FILENO);
		dup(conn_fd);
		char buf[1024];
		fgets(buf, sizeof(buf), stdin);
		log_info("%s\n", buf);
		close(conn_fd);
	}	
	return 0;
}
