#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/util/parse.h"
#include "../../src/util/config.h"

int main (int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "requires test path\n");
		return 1;
	}

	_CONFIG_T *config = malloc(sizeof(_CONFIG_T));
	memset(config, 0, sizeof(_CONFIG_T));
	if (parse_config_file(config, argv[1]) != 0) {
		fprintf(stderr, "error getting config file\n");
		return 1;
	}
	printf("user: %s, group: %s\n", config->user, config->group);
	printf("pid: %s\n", config->pidfile);
	printf("logs -- facility: %s, level: %s\n", config->log.facility, config->log.level);

	return 0;
}
