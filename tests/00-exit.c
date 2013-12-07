#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <pcm.h>

int main(int argc, char **argv)
{
	pcm_load_policy_from_file("../tests/00-exit.json", NULL);
	pcm_install_policy();

	return 0;
}
