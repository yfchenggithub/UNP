#include <string.h>
#include <netdb.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <errno.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <pthread.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

void log_debug(const char* fmt, ...)
{
	va_list	ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
}


#define BUFSIZE 1500
char sendbuf[BUFSIZE];
int data_len = 56;
char* host = NULL;
int nsent = 0;
pid_t pid = 0;
int sockfd = 0;

uint16_t in_chsum(uint16_t* addr, int len)
{
	int nleft = len;
	uint32_t sum  = 0;
	uint16_t* w = addr;
	uint16_t answer = 0;
	while (nleft > 1)
	{
		sum += *w++;
		nleft -= 2;
	}
	
	if (1 == nleft)
	{
		*(unsigned char*)(&answer) = *(unsigned char*)w;
		sum += answer;
	}
	
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	answer = ~sum;
	return answer;
}

struct proto
{
	void (*fproc)(char*, ssize_t, struct msghdr*, struct timeval*);
	void (*fsend)(void);
	void (*finit)(void);
	struct sockaddr* sa_send;
	struct sockaddr* sa_recv;
	socklen_t sa_len;
	int icmp_proto;
}*pr;

void tv_sub(struct timeval* out, struct timeval* in)
{
	if ((out->tv_usec -= in->tv_usec) < 0)
	{
		--out->tv_sec;
		out->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}


void proc_v4(char* ptr, ssize_t len, struct msghdr* msg, struct timeval* tvrecv)
{
	log_debug("enter proc_v4\n");
	int hlen1;
	int icmplen;
	double rtt;
	struct ip* pip;
	struct icmp* picmp;
	struct timeval* tvsend;
	
	pip = (struct ip*)ptr;
	hlen1 = pip->ip_hl << 2;
	if (IPPROTO_ICMP != pip->ip_p)
	{
		log_debug("not icmp msg\n");
		return;
	}	
	
	picmp = (struct icmp*)(ptr + hlen1);
	if ((icmplen = len - hlen1) < 8)
	{
		log_debug("malfromed packet\n");
		return;
	}
	
	if (ICMP_ECHOREPLY == picmp->icmp_type)
	{
		if (pid != picmp->icmp_id)
		{
			log_debug("pid not match\n");
			return;
		}
		tvsend = (struct timeval*)(picmp->icmp_data);
		tv_sub(tvrecv, tvsend);
		rtt = tvrecv->tv_sec * 1000.0 + (tvrecv->tv_usec / 1000.0);
		log_debug("%d bytes from %s: seq = %u, ttl = %d, rtt=%.3f ms\n", icmplen, host, picmp->icmp_seq, pip->ip_ttl, rtt);	
	}
}

void send_v4(void)
{
	log_debug("enter send_v4\n");
	int len;
	struct icmp* picmp;
	picmp = (struct icmp*)sendbuf;
	picmp->icmp_type = ICMP_ECHO;
	picmp->icmp_code = 0;
	picmp->icmp_id = pid;
	picmp->icmp_seq = nsent++;
	
	memset(picmp->icmp_data, 0xa5, data_len);
	gettimeofday((struct timeval*)picmp->icmp_data, NULL);
	
	len = 8 + data_len;
	picmp->icmp_cksum = 0;
	picmp->icmp_cksum = in_chsum((u_short*)picmp, len);
	int nbyte_send = sendto(sockfd, sendbuf, len, 0, pr->sa_send, pr->sa_len);
	if (-1 == nbyte_send)
	{
		fprintf(stderr, "%s\n", strerror(errno));
	}
	else
	{
		log_debug("send %d bytes\n", nbyte_send);
	}
}


void sig_alarm(int sig)
{
	log_debug("%s\n", strsignal(sig));
	(* pr->fsend)();
	alarm(2);
	return;
}

struct proto proto_v4 = {proc_v4, send_v4, NULL, NULL, NULL, 0, IPPROTO_ICMP};

void readloop(void)
{
	log_debug("enter readloop\n");
	int size;
	char recvbuf[BUFSIZE];
	char control_buf[BUFSIZE];
	struct msghdr msg;
	struct iovec iov;
	ssize_t n;
	struct timeval tval;
	sockfd = socket(pr->sa_send->sa_family, SOCK_RAW, pr->icmp_proto);
	if (sockfd < 0)
	{
		fprintf(stderr, "%s\n", strerror(errno));
	}
	else
	{
		log_debug("sockfd %d\n", sockfd);
	}
	setuid(getuid());
	if (pr->finit)
	{
		(*pr->finit)();
	}
	
	size = 60 * 1024;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
	sig_alarm(SIGALRM);
	
	iov.iov_base = recvbuf;
	iov.iov_len = sizeof(recvbuf);
	
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = control_buf;
	msg.msg_controllen = sizeof(control_buf);
	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	
	for (;;)
	{
		n = recvmsg(sockfd, &msg, 0);
		gettimeofday(&tval, NULL);
		(*pr->fproc)(recvbuf, n, &msg, &tval);
		sleep(3);
	}		
}	

int main(int argc, char** argv)
{
	if (2 != argc)
	{
		log_debug("usage: %s host\n", argv[0]);
		return -1;
	}

	host = argv[1];
	log_debug("host %s\n", host);
	pid = getpid() & 0xffff;
	
	log_debug("pid %ld\n", pid);
	struct sigaction new_action;
	new_action.sa_flags = 0;
	sigemptyset(&new_action.sa_mask);
	new_action.sa_handler = sig_alarm;
	if (sigaction(SIGALRM, &new_action, NULL) < 0)
	{
		log_debug("instal SIGALRM fail\n");
		return -1;
	}	
	
	log_debug("ping %s %d bytes\n", host, data_len);
	
	struct addrinfo addr_hints;
	addr_hints.ai_family = AF_INET;
	addr_hints.ai_flags = 0;
	addr_hints.ai_socktype = SOCK_STREAM;
	addr_hints.ai_protocol = 0;
	
	struct addrinfo* ai;
	if (0 != getaddrinfo(host, NULL, &addr_hints, &ai))
	{
		log_debug("getaddrinfo fail\n");
		return -1;
	}
	log_debug("getaddrinfo succ\n");
	
	if (ai->ai_family = AF_INET)
	{
		log_debug("addr family AF_INET \n");
		pr = &proto_v4;
	}

	pr->sa_send = ai->ai_addr;
	pr->sa_recv = calloc(1, ai->ai_addrlen);
	pr->sa_len = ai->ai_addrlen;

	readloop();
	return 0;
}
