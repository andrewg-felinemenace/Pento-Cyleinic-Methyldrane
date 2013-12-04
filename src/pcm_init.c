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

void convert_slashes_to_underlines(char *buf)
{
	char *p;

	p = buf;
	while(*p) {
		if(*p == '/') *p = '_';
		p++;
	}	
}

void pcm_initialize()
{
	char *path;
	char buf[4096];
	int plus_one;

	// PCM_POLICY_FILE => file to use
	// PCM_HOOK => start,setuid,accept

	// if no PCM_POLICY_FILE, then don't set PCM_HOOK.

	PCM_GLOBAL.policy_file = getenv("PCM_POLICY_FILE");
	if(PCM_GLOBAL.policy_file) {
		PCM_GLOBAL.hook = getenv("PCM_HOOK");
		if(PCM_GLOBAL.hook && strcmp(PCM_GLOBAL.hook, "start") == 0) {
			pcm_load_policy_from_file(PCM_GLOBAL.policy_file);
		}
		return;
	} 

	memset(buf, 0, sizeof(buf));

	// not much we can do if readlink errors!
	if(readlink("/proc/self/exe", buf, sizeof(buf)-1) == -1) {
		// XXX, this is an ugly hack for now.
		if(getpid() == 1) {
			strcpy(buf, "/sbin/init");
		} else {
			return;
		}
	}
	
	plus_one = (buf[0] == '/');
	convert_slashes_to_underlines(buf);
	
	asprintf(&path, "/etc/pcm/%s.json", buf + plus_one);

	// pcm_parse_policy_from_file(path);
	
	free(path);
}
