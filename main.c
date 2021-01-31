#include <stdlib.h>

#include "x.h"

int
main() {
	if (init_window())
		exit(1);

	while (1) {
		parse_events();
	}

	return 0;
}
