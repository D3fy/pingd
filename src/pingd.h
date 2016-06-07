
#ifndef _PINGD_H
#define _PINGD_H
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <fcntl.h>
#include <errno.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>


#define PACKETSIZE 128
struct packet
{
	struct icmphdr hdr;
	char msg[PACKETSIZE - sizeof(struct iphdr) - sizeof(struct icmphdr)];
};

int pid = -1;
struct protoent *proto=NULL;

unsigned short checksum(void *b, int len);

void display(void *buf, int bytes);

void listener(void);

void ping(struct sockaddr_in *addr);

#endif
