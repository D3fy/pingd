#include "pingd.h"


unsigned short checksum(void *b, int len) /* {{{ */
{
	unsigned short *buf = b;
	unsigned int sum=0;
	unsigned short result;

	for (sum = 0; len > 1; len -= 2 )
		sum += *buf++;
	if (len == 1 )
		sum += *(unsigned char*)buf;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	result = ~sum;
	return result;
}
/* }}} */

void display(void *buf, int pid, unsigned long long ret) /* {{{ */
{
	struct iphdr   *ip   = buf;
	struct icmphdr *icmp = buf + ip->ihl * 4;

	int hdr_size = sizeof(struct iphdr) + sizeof(struct icmphdr);
	unsigned long send_time_sec, send_time_nanosec;
	memcpy(&send_time_sec, ((char *)buf) + hdr_size, 4);
	memcpy(&send_time_nanosec, ((char *)buf) + hdr_size + 4, 4);
	char host[56];
	memcpy(host, ((char *)buf) + hdr_size + 8, 56);
	for (int i = 0; i < 56; i++)
		host[i] = host[i] - '0';
	host[56 - 1]      = '\0';
	send_time_sec     = ntohl(send_time_sec);
	send_time_nanosec = ntohl(send_time_nanosec);
	printf("rtt[%llu us]", ret - ((unsigned long long) send_time_sec * 1000000 + (unsigned long long) send_time_nanosec / 1000));
	printf(" seq[%u]", ntohs(icmp->un.echo.sequence));
	printf(" id[%d]",  ntohs(icmp->un.echo.id));
	printf(" src[%s]",
		inet_ntoa(*((struct in_addr *)&((ip->saddr)))));
	if (ntohs(icmp->un.echo.id) != pid)
		printf(" FAILED PING");
	printf("\n");
}
/* }}} */

void listener(int pid, struct protoent *proto) /* {{{ */
{
	int sd;
	struct sockaddr_in addr;
	unsigned char buf[1024];

	if ((sd = socket(PF_INET, SOCK_RAW|SOCK_NONBLOCK, proto->p_proto)) < 0 ) {
		perror("socket");
		exit(0);
	}
	// fcntl(sd, F_SETFL, O_NONBLOCK);
	struct epoll_event ev, events[MAX_EVENTS];
	int nfds, epollfd;
	struct timespec t;

	ev.events = EPOLLIN; // |EPOLLET;
	if ((epollfd = epoll_create1(0)) == -1)
		fprintf(stderr, "epoll_create1");
	ev.data.fd = sd;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sd, &ev) == -1)
		fprintf(stderr, "epoll_ctl sfd");
	for (;;) {
		if ((nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1)) == -1)
			fprintf(stderr, "epoll_wait");

		for (int n = 0; n < nfds; ++n) {
			if (events[n].data.fd == sd) {
				int bytes;
				unsigned int len = sizeof(addr);

				memset(buf, 0, sizeof(buf));
				clock_gettime(CLOCK_REALTIME, &t);
				unsigned long long ret = t.tv_sec * 1000000 + t.tv_nsec / 1000;
				bytes = recvfrom(sd, buf, sizeof(buf), 0, (struct sockaddr*) &addr, &len);
				if (bytes > 0)
					display(buf, pid, ret);
				else
					perror("recvfrom");
			}
		}
	}
	exit(0);
}
/* }}} */

void ping(char *host, struct sockaddr_in *addr, int pid, struct protoent *proto, unsigned short cnt) /* {{{ */
{
	const int ttl  = 61;
	int sd;
	struct packet pckt;
	struct sockaddr_in r_addr;

	if ((sd = socket(PF_INET, SOCK_RAW|SOCK_NONBLOCK, proto->p_proto)) < 0) {
		perror("socket");
		return;
	}
	if (setsockopt(sd, SOL_IP, IP_TTL, &ttl, sizeof(ttl)) != 0)
		perror("Set TTL option");
	if (fcntl(sd, F_SETFL, O_NONBLOCK) != 0 )
		perror("Request nonblocking I/O");

	unsigned int len = sizeof(r_addr);

	memset(&pckt, 0, sizeof(pckt));
	pckt.hdr.type = ICMP_ECHO;
	pckt.hdr.un.echo.id = htons(pid);

	unsigned char hst[56];
	memcpy(hst, host, 56);
	for (int i = 0; i < 56; i++)
		hst[i] = hst[i] + '0';
	struct timespec t;
	clock_gettime(CLOCK_REALTIME, &t);
	unsigned long org_s  = htonl(t.tv_sec);
	unsigned long org_ns = htonl(t.tv_nsec);
	memcpy(pckt.msg,     &org_s,  4);
	memcpy(pckt.msg + 4, &org_ns, 4);
	memcpy(pckt.msg + 8, hst, 56);

	pckt.hdr.un.echo.sequence = htons(cnt);
	pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));

	if (sendto(sd, &pckt, sizeof(pckt), 0, (struct sockaddr*)addr, sizeof(*addr)) <= 0 )
		perror("sendto");
}
/* }}} */
