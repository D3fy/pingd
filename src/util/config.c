#include "config.h"
#include <string.h>
#include <stdlib.h>

void add_host(_CONFIG_T *conf, char *name, char *address)
{
	conf->hosts[conf->hosts_len]          = malloc(sizeof(_host));
	conf->hosts[conf->hosts_len]->name    = strdup(name);
	conf->hosts[conf->hosts_len]->address = strdup(address);
	conf->hosts_len++;
}

_host pop_host(_CONFIG_T *conf)
{
	_host h = { 0 };
	if (conf->hosts_len <= 0)
		return h;
	h.name    = conf->hosts[conf->hosts_len]->name;
	h.address = conf->hosts[conf->hosts_len]->address;
	free(conf->hosts[conf->hosts_len]);
	conf->hosts_len--;
	return h;
}

_host free_host(_CONFIG_T *conf, int idx)
{
	_host h = { 0 };
	h.name    = conf->hosts[idx]->name;
	h.address = conf->hosts[idx]->address;
	free(conf->hosts[idx]);
	return h;
}
