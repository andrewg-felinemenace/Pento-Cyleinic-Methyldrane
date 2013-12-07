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
	int rc;
	char *hook;

	// PCM_POLICY_FILE => file to use
	// PCM_HOOK => start,setuid,accept

	// if no PCM_POLICY_FILE, then don't set PCM_HOOK.

	hook = NULL;
	path = getenv("PCM_POLICY_FILE");
	if(path) {
		rc = pcm_load_policy_from_file(path, &hook);

		if(! rc) {
			char *envhook;
			char *usehook;
			envhook = getenv("PCM_HOOK");

			usehook = envhook ? envhook : hook;

			pcm_hook_set(usehook);	// if we set a NULL hook, it will just free the policy memory.
			return;
		}
	} 

	memset(buf, 0, sizeof(buf));

	// not much we can do if readlink errors!
	if(readlink("/proc/self/exe", buf, sizeof(buf)-1) == -1) {
		// XXX, this is an ugly hack for now.
		if(getpid() == 1) {
			strcpy(buf, "/sbin/init");
		} 
	}
	
	plus_one = (buf[0] == '/') ? 1 : 0;
	convert_slashes_to_underlines(buf);
	
	asprintf(&path, "/etc/pcm/%s.json", buf + plus_one);

	hook = NULL;
	rc = pcm_try_load_policy_from_file(path, &hook);
	// printf("got %d from try_load and hook is %s\n", rc, hook);
	pcm_hook_set(hook);
	
	free(path);
}
