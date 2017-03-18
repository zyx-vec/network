#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define MAXSIZE 64

int main() {
	char buff[MAXSIZE];

	time_t ticks;

	ticks = time(NULL);
	snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
	printf("%s", buff);
	// default at least 10s, or you could append a number with
	// suffix like `m` and `h` to stand for minute or hour.
	sleep(3);

	ticks = time(NULL);
	snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
	printf("%s", buff);

	return 0;
}
