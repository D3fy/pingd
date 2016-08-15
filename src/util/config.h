#ifndef _PARSE_CONFIG
#define _PARSE_CONFIG

typedef struct {
	char *name;
	char *address;
} _host;

typedef struct {
	char *user;
	char *group;
	char *pidfile;
	struct {
		char *facility;
		char *level;
	} log;
	_host **hosts;
	int hosts_len;
} _CONFIG_T;

int parse_config_file(_CONFIG_T *conf, const char *path);

void add_host(_CONFIG_T *conf, char *name, char *address);
_host pop_host(_CONFIG_T *conf);
_host free_host(_CONFIG_T *conf, int idx);


#endif
