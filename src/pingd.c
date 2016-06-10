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

void display(void *buf, int bytes, int pid, unsigned long long ret) /* {{{ */
{
	struct iphdr   *ip   = buf;
	struct icmphdr *icmp = buf + ip->ihl * 4;

	printf("\n--- data start ---\n");
	int hdr_size = sizeof(struct iphdr) + sizeof(struct icmphdr);
	unsigned long org_s, org_ns;
	memcpy(&org_s, ((char *)buf) + hdr_size, 4);
	memcpy(&org_ns, ((char *)buf) + hdr_size + 4, 4);
	char host[56];
	memcpy(host, ((char *)buf) + hdr_size + 8, 56);
	for (int i = 0; i < 56; i++)
		host[i] = host[i] - '0';
	host[56 - 1] = '\0';
	org_s = ntohl(org_s);
	org_ns = ntohl(org_ns);
	printf("rtt: %llu us\n", ret - (org_s * 1000000 + org_ns / 1000));
	printf("host: %s\n", host);
	//for (int i = hdr_size; i < bytes - hdr_size; i++)
	//	printf("%c", ((char*)buf)[i] - '0');
	printf("--- data finished ---\n");

	printf("\nIPv%d: hdr-size=%d pkt-size=%d protocol=%d TTL=%d src=%s ",
		ip->version, ip->ihl*4, ntohs(ip->tot_len), ip->protocol,
		ip->ttl,
		inet_ntoa(*((struct in_addr *)&((ip->saddr)))));

	printf("dst=%s\n", inet_ntoa(*((struct in_addr *)&((ip->daddr)))));

	if (ntohs(icmp->un.echo.id) == pid) {
		printf("ICMP: type[%d/%u] checksum[%u] id[%d] seq[%d]\n\n\n",
			icmp->type, icmp->code, ntohs(icmp->checksum),
			ntohs(icmp->un.echo.id), ntohs(icmp->un.echo.sequence));
	}
}
/* }}} */

void listener(int pid, struct protoent *proto) /* {{{ */
{
	int sd;
	struct sockaddr_in addr;
	unsigned char buf[1024];

	if ((sd = socket(PF_INET, SOCK_RAW, proto->p_proto)) < 0 ) {
		perror("socket");
		exit(0);
	}
	for (;;) {
		int bytes;
		unsigned int len = sizeof(addr);

		memset(buf, 0, sizeof(buf));
		bytes = recvfrom(sd, buf, sizeof(buf), 0, (struct sockaddr*) &addr, &len);
		struct timespec t;
		clock_gettime(CLOCK_REALTIME, &t);
		unsigned long long ret = t.tv_sec * 1000000 + t.tv_nsec / 1000;
		if (bytes > 0)
			display(buf, bytes, pid, ret);
		else
			perror("recvfrom");
	}
	exit(0);
}
/* }}} */

void ping(char *host, struct sockaddr_in *addr, int pid, struct protoent *proto) /* {{{ */
{
	const int ttl  = 61;
	int sd;
	unsigned int cnt = 0;
	struct packet pckt;
	struct sockaddr_in r_addr;

	if ((sd = socket(PF_INET, SOCK_RAW, proto->p_proto)) < 0) {
		perror("socket");
		return;
	}
	if (setsockopt(sd, SOL_IP, IP_TTL, &ttl, sizeof(ttl)) != 0)
		perror("Set TTL option");
	if (fcntl(sd, F_SETFL, O_NONBLOCK) != 0 )
		perror("Request nonblocking I/O");
	for (;;) {
		unsigned int len = sizeof(r_addr);

		printf("Msg #%d\n", cnt);
		if (recvfrom(sd, &pckt, sizeof(pckt), 0, (struct sockaddr*)&r_addr, &len) < 0 )
			perror("failed recieve");
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
		pckt.hdr.un.echo.sequence = htons(cnt++);
		pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));
		if (sendto(sd, &pckt, sizeof(pckt), 0, (struct sockaddr*)addr, sizeof(*addr)) <= 0 )
			perror("sendto");
		sleep(1);
	}
}
/* }}} */
