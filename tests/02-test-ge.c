#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <pcm.h>

int main(int argc, char **argv)
{
	int fd;
	char c;

	fd = open("/etc/passwd", O_RDONLY);

	setenv("PCM_HOOK", "start", 1);
	setenv("PCM_POLICY_FILE", "../tests/02-test-ge.json", 1);
	pcm_initialize();

	if(read(fd, &c, 1) != 1) {
		errx(EXIT_FAILURE, "Failed to read from fd: %m");
	}

	return 0;
}
