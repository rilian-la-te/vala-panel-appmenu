#include "dbusmenu-definitions.h"
#include <stdlib.h>

int string_hash(const signed char *v)
{
	const signed char *p;
	int h = 5381;

	for (p = v; *p != '\0'; p++)
		h = (h << 5) + h + *p;
	return abs(h);
}
