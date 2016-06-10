#include "pingd.h"
#include "util/logger.h"
#include "util/daemonize.h"
#include <sys/timerfd.h>

int main (int argc, char *strings[])
{
	struct hostent *hname;
	struct sockaddr_in addr;
	struct protoent *proto = NULL;
	int    pid = -1;

	if (argc != 2) {
		printf("usage: %s <addr>\n", strings[0]);
		exit(0);
	}

	pid   = getpid();
	proto = getprotobyname("ICMP");
	hname = gethostbyname(strings[1]);
	memset(&addr, 0, sizeof(addr));

	addr.sin_family = hname->h_addrtype;
	addr.sin_port   = 0;
	addr.sin_addr.s_addr = *(long *)hname->h_addr;

	if (fork() == 0)
		listener(pid, proto);

	int fd = timerfd_create(CLOCK_REALTIME, 0);
	fcntl(fd, F_SETFL, O_NONBLOCK);
	struct itimerspec it;
	it.it_interval.tv_sec  = 0;
	it.it_interval.tv_nsec = 2 * 100 * 1000 * 1000;
	it.it_value.tv_sec     = 0;
	it.it_value.tv_nsec    = 2 * 100 * 1000 * 1000;

	struct epoll_event ev, events[MAX_EVENTS];
	int nfds, epollfd;

	ev.events = EPOLLIN| EPOLLET; 
	uint64_t value;

	if ((epollfd = epoll_create1(0)) == -1)
		fprintf(stderr, "epoll_create1");
	timerfd_settime(fd, 0, &it, NULL);
	ev.data.fd = fd;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1)
		fprintf(stderr, "epoll_ctl sfd");

	for (;;) {
		if ((nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1)) == -1)
			fprintf(stderr, "epoll_wait");

		for (int n = 0; n < nfds; ++n) {
			if (events[n].data.fd == fd) {
				read(fd, &value, 8);
				ping(strings[1], &addr, pid, proto);
			}
		}
	}

	return 0;
}
