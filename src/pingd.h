
#ifndef _PINGD_H
#define _PINGD_H
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

#include <fcntl.h>
#include <errno.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>


#define PACKETSIZE 20
struct packet
{
	struct icmphdr hdr;
	unsigned char msg[12];
};

unsigned short checksum(void *b, int len);

void display(void *buf, int bytes, int pid);

void listener(int pid, struct protoent *proto);

void ping(struct sockaddr_in *addr, int pid, struct protoent *proto);

#endif
