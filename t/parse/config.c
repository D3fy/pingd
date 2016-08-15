#include <stdio.h>
#include "../../src/util/config.h"

int main (int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "requires test path\n");
		return 1;
	}

	_CONFIG_T config
	if (get_config_file(&config, argv[1]) != 0) {
		fprintf(stderr, "error getting config file\n");
		return 1;
	}

	return 0;
}
