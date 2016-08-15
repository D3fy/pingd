#include "pingd.h"

#include <time.h>
#include <sys/timerfd.h>
#include <getopt.h>

#include "util/daemonize.h"
#include "util/config.h"


static struct
{
	int   verbose;
	char *facility;
	int   daemonize;
	char *pidfile;
	char *config;
	char *user;
	char *group;
} OPTIONS = { 0 };

int main (int argc, char *argv[])
{
	OPTIONS.facility  = strdup("daemon");
	OPTIONS.verbose   = 0;
	OPTIONS.daemonize = 1;
	OPTIONS.config    = strdup("/etc/pingd.conf");
	OPTIONS.pidfile   = strdup("/run/pingd.pid");
	OPTIONS.user      = strdup("root");
	OPTIONS.group     = strdup("root");
	struct option long_opts[] = {
		{ "help",             no_argument, NULL, 'h' },
		{ "verbose",          no_argument, NULL, 'v' },
		{ "foreground",       no_argument, NULL, 'F' },
		{ "config",     required_argument, NULL, 'c' },
		{ 0, 0, 0, 0 },
	};
	for (;;) {
		int idx = 1;
		int c = getopt_long(argc, argv, "h?v+Fc:", long_opts, &idx);
		if (c == -1) break;

		switch (c) {
		case 'h':
		case '?':
			exit(0);
		case 'v':
			OPTIONS.verbose++;
			break;

		case 'F':
			OPTIONS.daemonize = 0;
			break;

		case 'c':
			free(OPTIONS.config);
			OPTIONS.config = strdup(optarg);
			break;
		default:
			fprintf(stderr, "unhandled option flag %#02x\n", c);
			return 1;
		}
	}

	_CONFIG_T *conf = malloc(sizeof(_CONFIG_T));
	memset(conf, 0, sizeof(_CONFIG_T));
	conf->hosts = malloc(sizeof(_host *));
	if (parse_config_file(conf, OPTIONS.config) != 0)
		return 1;

	if (OPTIONS.daemonize) {
		log_open("pingd", OPTIONS.facility);
		log_level(LOG_NOTICE + OPTIONS.verbose, NULL);

		mode_t um = umask(0);
		if (daemonize(OPTIONS.pidfile, OPTIONS.user, OPTIONS.group) != 0) {
			fprintf(stderr, "daemonization failed: (%i) %s\n", errno, strerror(errno));
			return 3;
		}
		umask(um);
	} else {
		log_open("pingd", "console");
		log_level(LOG_NOTICE + OPTIONS.verbose, NULL);
		if (!freopen("/dev/null", "r", stdin))
			logger(LOG_WARNING, "failed to reopen stdin </dev/null: %s", strerror(errno));
	}

	struct hostent *hname;
	struct sockaddr_in **addr = malloc(sizeof(struct sockaddr_in *) * argc);

	int pid = getpid();
	struct protoent *proto =  getprotobyname("ICMP");

	// for (int i = 1; i < argc; i++) {
	for (int i = 0; i < conf->hosts_len; i++) {
		hname = gethostbyname(conf->hosts[i]->address);
		addr[i] = malloc(sizeof(struct sockaddr_in));
		memset(addr[i], 0, sizeof(struct sockaddr_in));
		addr[i]->sin_family = hname->h_addrtype;
		addr[i]->sin_port   = 0;
		addr[i]->sin_addr.s_addr = *(long *)hname->h_addr;
	}
	for (int hostidx = 1; hostidx < argc; hostidx++) {

	}


	int fd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK);
	int child_pid;
	if ((child_pid = fork()) == 0)
		listener(pid, proto);
	logger(LOG_INFO, "starting up");

	struct itimerspec it;
	it.it_interval.tv_sec  = 5;
	it.it_interval.tv_nsec = 0;
	it.it_value.tv_sec     = it.it_interval.tv_sec;
	it.it_value.tv_nsec    = it.it_interval.tv_nsec;

	struct epoll_event ev, events[MAX_EVENTS];
	int nfds, epollfd;

	ev.events = EPOLLIN|EPOLLET;
	uint64_t value;

	if ((epollfd = epoll_create1(0)) == -1) {
		logger(LOG_ERR, "epoll_create1: %s", strerror(errno));
		exit(1);
	}

	timerfd_settime(fd, 0, &it, NULL);

	ev.data.fd = fd;

	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		logger(LOG_ERR, "epoll_ctl failed to add timerfd: %s", strerror(errno));
		exit(1);
	}
	unsigned short seq[conf->hosts_len];
	for (int i = 0; i < conf->hosts_len; i++)
		seq[i] = 0;

	for (;;) {
		if ((nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1)) == -1)
			logger(LOG_WARNING, "epoll_wait error %s", strerror(errno));

		for (int n = 0; n < nfds; ++n) {
			if (events[n].data.fd == fd) {
				if (read(fd, &value, sizeof(uint64_t)) == -1) {
					logger(LOG_ERR, "failed to read timer, %s", strerror(errno));
					exit(1);
				}
				if (value > 1)
					logger(LOG_WARNING, "missed timmer interval");
				if (value == 0)
					logger(LOG_WARNING, "double timer read");
				for (int z = 0; z < conf->hosts_len; z++)
					ping(addr[z], pid, proto, seq[z]++, (unsigned short) z);
			}
		}
	}

	return 0;
}
