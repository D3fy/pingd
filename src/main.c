#include "pingd.h"

int main (int argc, char *strings[])
{
	struct hostent *hname;
	struct sockaddr_in addr;

	if (argc != 2) {
		printf("usage: %s <addr>\n", strings[0]);
		exit(0);
	}

	pid = getpid();
	proto = getprotobyname("ICMP");
	hname = gethostbyname(strings[1]);
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = hname->h_addrtype;
	addr.sin_port = 0;
	addr.sin_addr.s_addr = *(long*)hname->h_addr;

	if (fork() == 0)
		listener();
	else
		ping(&addr);

	wait(0);
	return 0;
}
