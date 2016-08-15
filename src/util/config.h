#ifndef _PARSE_CONFIG
#define _PARSE_CONFIG

typedef struct {
	char *name;
	char *address;
} host;

typedef struct {
	char *user;
	char *group;
	char *pidfile;
	struct {
		char *facility;
		char *level;
	} log;
	host **hosts;
	int hosts_len;
} _CONFIG_T;


int parse_config_file (_CONFIG_T *conf, const char *path);

#endif
