#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>

#include "pcm.h"

void pcm_initialize() __attribute__((constructor));

struct pcm_global PCM_GLOBAL;

void pcm_initialize()
{
	// PCM_POLICY_FILE => file to use
	// PCM_HOOK => start,setuid,accept

	// if no PCM_POLICY_FILE, then don't set PCM_HOOK.

	PCM_GLOBAL.policy_file = getenv("PCM_POLICY_FILE");
	if(! PCM_GLOBAL.policy_file) return;
	PCM_GLOBAL.hook = getenv("PCM_HOOK");
}
