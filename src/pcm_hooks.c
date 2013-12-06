#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <pcm.h>

#define __USE_GNU // for RTLD_NEXT
#include <dlfcn.h>

int (*accept_fp)(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int (*connect_fp)(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int (*setuid_fp)(uid_t uid);

void pcm_resolve_symbols() __attribute__((constructor));

int pcm_hook_initialized;

void pcm_resolve_symbols()
{
	accept_fp = dlsym(RTLD_NEXT, "accept");
	setuid_fp = dlsym(RTLD_NEXT, "setuid");
	connect_fp = dlsym(RTLD_NEXT, "connect");
}

void pcm_hook_set(char *hook)
{
	PCM_GLOBAL.hook = hook;

	if(! hook) {
		pcm_policy_free();
		return;		
	}

	if(strcmp(hook, "start") == 0) {
		pcm_install_policy();
	}

}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	int ret;

	ret = accept_fp(sockfd, addr, addrlen);

	if(!pcm_hook_initialized && PCM_GLOBAL.hook && strcmp(PCM_GLOBAL.hook, "accept") == 0) {
		pcm_hook_initialized = 1;
		pcm_install_policy();
	}

	return ret;
}

int setuid(uid_t uid)
{
	int ret;
	// drop privileges first, so that setuid isn't a required system call!
	ret = setuid_fp(uid);

	if(!pcm_hook_initialized && PCM_GLOBAL.hook && strcmp(PCM_GLOBAL.hook, "setuid") == 0) {
		pcm_hook_initialized = 1;
		pcm_install_policy();
	}

	return ret;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	int ret;

	ret = connect_fp(sockfd, addr, addrlen);

	if(!pcm_hook_initialized && PCM_GLOBAL.hook && strcmp(PCM_GLOBAL.hook, "connect") == 0) {
		pcm_hook_initialized = 1;
		pcm_install_policy();
	}

	return ret;
}
