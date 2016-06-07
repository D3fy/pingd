#include pingd.h


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

void display(void *buf, int bytes) /* {{{ */
{
	struct iphdr   *ip   = buf;
	struct icmphdr *icmp = buf + ip->ihl * 4;

	printf("\n--- data start ---\n");
	int hdr_size = sizeof(struct iphdr) + sizeof(struct icmphdr);
	for (int i = hdr_size; i < bytes - hdr_size; i++)
		printf("%c", ((char*)buf)[i] - '0');
	printf("\n--- data finished ---\n");

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

void listener(void) /* {{{ */
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
		if (bytes > 0)
			display(buf, bytes);
		else
			perror("recvfrom");
	}
	exit(0);
}
/* }}} */

void ping(struct sockaddr_in *addr) /* {{{ */
{
	const int val  = 255;
	int i, sd, cnt = 1;
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
		pckt.hdr.type = ICMP_ECHO;
		pckt.hdr.un.echo.id = pid;
		char *message = malloc(PACKETSIZE);
		memset(message, 0, PACKETSIZE);
		strncpy(message, "Hello Google", 13);
		for (i = 0; i < PACKETSIZE - 1; i++)
			pckt.msg[i] = message[i] + '0';
		pckt.msg[PACKETSIZE - 1] = '\0';
		pckt.msg[PACKETSIZE] = 0;
		pckt.hdr.un.echo.sequence = cnt++;
		pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));
		if (sendto(sd, &pckt, sizeof(pckt), 0, (struct sockaddr*)addr, sizeof(*addr)) <= 0 )
			perror("sendto");
		sleep(1);
	}
}
/* }}} */
