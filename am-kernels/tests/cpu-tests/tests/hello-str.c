#include "trap.h"

char buf[128];

int main() {
	check(sprintf(buf, "%s %s", "Hello", "Hello") == 11);
	check(strcmp(buf, "Hello Hello") == 0);

	sprintf(buf, "%d + %d = %d\n", 1, 1, 2);
	check(strcmp(buf, "1 + 1 = 2\n") == 0);
	check(strcmp(buf, "1 + 1 = 3\n") < 0);

	sprintf(buf, "%d + %d = %d\n", -2, 12, 10);
	check(strcmp(buf, "-2 + 12 = 10\n") == 0);
	return 0;
}
