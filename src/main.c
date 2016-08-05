#include "pingd.h"
#include "util/logger.h"
#include "util/daemonize.h"
#include <sys/timerfd.h>

int main (int argc, char *argv[])
{
	struct hostent *hname;
	struct sockaddr_in **addr = malloc(sizeof(struct sockaddr_in *) * argc);
	struct protoent *proto = NULL;
	int    pid = -1;

	if (argc < 2) {
		printf("usage: %s <addr> ...\n", argv[0]);
		exit(0);
	}

	pid   = getpid();
	proto = getprotobyname("ICMP");

	for (int i = 1; i < argc; i++) {
		hname = gethostbyname(argv[i]);
		addr[i] = malloc(sizeof(struct sockaddr_in));
		memset(addr[i], 0, sizeof(struct sockaddr_in));
		addr[i]->sin_family = hname->h_addrtype;
		addr[i]->sin_port   = 0;
		addr[i]->sin_addr.s_addr = *(long *)hname->h_addr;
	}

	int fd = timerfd_create(CLOCK_REALTIME, 0);
	int child_pid;
	if ((child_pid = fork()) == 0)
		listener(pid, proto);
	printf("parent[%d] child[%d]\n", pid, child_pid);

	fcntl(fd, F_SETFL, O_NONBLOCK);
	struct itimerspec it;
	it.it_interval.tv_sec  = 1;
	it.it_interval.tv_nsec = 0;// 2 * 100 * 1000 * 1000;
	it.it_value.tv_sec     = it.it_interval.tv_sec;
	it.it_value.tv_nsec    = it.it_interval.tv_nsec;

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
	unsigned short seq[argc];
	for (int i = 1; i < argc; i++)
		seq[i] = 0;

	for (;;) {
		if ((nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1)) == -1)
			fprintf(stderr, "epoll_wait");

		for (int n = 0; n < nfds; ++n) {
			if (events[n].data.fd == fd) {
				if (read(fd, &value, 8) == -1) {
					fprintf(stderr, "failed to read timer, %s\n", strerror(errno));
					exit(1);
				}
				for (int z = 1; z < argc; z++)
					ping(argv[z], addr[z], pid, proto, seq[z]++);
			}
		}
	}

	return 0;
}
