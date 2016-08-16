
#ifndef _PINGD_H
#define _PINGD_H
#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>

#include <fcntl.h>
#include <errno.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>

#include "util/logger.h"

// #include "util/linkedlist.h"

#define MAX_EVENTS 1024

struct _host
{
	int            timerfd;
	unsigned int   seq;
	char          *address;
	char          *name;
	// struct _llist *rtt;
};

struct packet
{
	struct icmphdr hdr;
	unsigned char msg[64];
};

unsigned short checksum(void *b, int len);

void display(void *buf, int pid, unsigned long long ret);

void listener(int pid, struct protoent *proto);

void ping(struct sockaddr_in *addr, int pid, struct protoent *proto, unsigned short seq, unsigned short hostid);

#endif
