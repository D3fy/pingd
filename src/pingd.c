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

void display(void *buf, int bytes, int pid, unsigned long ret) /* {{{ */
{
	struct iphdr   *ip   = buf;
	struct icmphdr *icmp = buf + ip->ihl * 4;

	printf("\n--- data start ---\n");
	int hdr_size = sizeof(struct iphdr) + sizeof(struct icmphdr);
	unsigned long org, rcv, snd;
	memcpy(&org, ((char *)buf) + hdr_size, 4);
	memcpy(&rcv, ((char *)buf) + hdr_size + 4, 4);
	memcpy(&snd, ((char *)buf) + hdr_size + 8, 4);
	org = ntohl(org);
	rcv = ntohl(rcv);
	snd = ntohl(snd);
	printf("org: %lu, rcv: %lu, snd: %lu, ret: %lu\n", org, rcv, snd, ret);
	printf("tx: %lu ms, rx: %lu ms\n", rcv - org, ret - snd);
	for (int i = hdr_size; i < bytes - hdr_size; i++)
		printf("%c", ((char*)buf)[i] - '0');
	printf("--- data finished ---\n");

	printf("\nIPv%d: hdr-size=%d pkt-size=%d protocol=%d TTL=%d src=%s ",
		ip->version, ip->ihl*4, ntohs(ip->tot_len), ip->protocol,
		ip->ttl,
		inet_ntoa(*((struct in_addr *)&((ip->saddr)))));

	printf("dst=%s\n", inet_ntoa(*((struct in_addr *)&((ip->daddr)))));

	if (icmp->un.echo.id == pid) {
		printf("ICMP: type[%d/%u] checksum[%u] id[%d] seq[%d]\n\n\n",
			icmp->type, icmp->code, ntohs(icmp->checksum),
			icmp->un.echo.id, icmp->un.echo.sequence);
	}
}
/* }}} */

void listener(int pid, struct protoent *proto) /* {{{ */
{
	int sd;
	struct sockaddr_in addr;
	unsigned char buf[1024];
	struct timeval t;

	if ((sd = socket(PF_INET, SOCK_RAW, proto->p_proto)) < 0 ) {
		perror("socket");
		exit(0);
	}
	for (;;) {
		int bytes;
		unsigned int len = sizeof(addr);

		memset(buf, 0, sizeof(buf));
		bytes = recvfrom(sd, buf, sizeof(buf), 0, (struct sockaddr*) &addr, &len);
		gettimeofday(&t, NULL);
		unsigned long ret = (t.tv_sec % 86400) * 1000 + t.tv_usec / 1000;
		if (bytes > 0)
			display(buf, bytes, pid, ret);
		else
			perror("recvfrom");
	}
	exit(0);
}
/* }}} */

void ping(struct sockaddr_in *addr, int pid, struct protoent *proto) /* {{{ */
{
	const int val  = 5;
	int sd, cnt = 0;
	struct packet pckt;
	struct sockaddr_in r_addr;

	if ((sd = socket(PF_INET, SOCK_RAW, proto->p_proto)) < 0) {
		perror("socket");
		return;
	}
	if (setsockopt(sd, SOL_IP, IP_TTL, &val, sizeof(val)) != 0)
		perror("Set TTL option");
	if (fcntl(sd, F_SETFL, O_NONBLOCK) != 0 )
		perror("Request nonblocking I/O");
	for (;;) {
		unsigned int len = sizeof(r_addr);

		printf("Msg #%d\n", cnt);
		if (recvfrom(sd, &pckt, sizeof(pckt), 0, (struct sockaddr*)&r_addr, &len) < 0 )
			perror("failed recieve");
		memset(&pckt, 0, sizeof(pckt));
		pckt.hdr.type = ICMP_TIMESTAMP;
		pckt.hdr.un.echo.id = pid;
		struct timeval t;
		gettimeofday(&t, NULL);
		unsigned long org = (t.tv_sec % 86400) * 1000 + t.tv_usec / 1000;
		printf("send org: %lu\n", org);
		org = htonl(org);
		memcpy(pckt.msg, &org, 4);
		pckt.hdr.un.echo.sequence = cnt++;
		pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));
		if (sendto(sd, &pckt, sizeof(pckt), 0, (struct sockaddr*)addr, sizeof(*addr)) <= 0 )
			perror("sendto");
		sleep(1);
	}
}
/* }}} */
