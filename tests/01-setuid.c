#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <pcm.h>

int main(int argc, char **argv)
{
	setenv("PCM_HOOK", "setuid", 1);
	setenv("PCM_POLICY_FILE", "../tests/01-setuid.json", 1);
	pcm_initialize();

	setuid(getuid());
	return 0;
}
