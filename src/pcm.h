#include <seccomp.h>
#include <jansson.h>

// src/pcm_init.c

struct pcm_global {
        char *hook;
	json_t *policy;
	scmp_filter_ctx seccomp;
};

extern struct pcm_global PCM_GLOBAL;

// src/pcm_policy.c
int pcm_try_load_policy_from_file(char *filename, char **hook);
void pcm_load_policy_from_file(char *filename);
void pcm_policy_free();
void pcm_install_policy();

// src/pcm_hooks.c

void pcm_hook_set(char *hook);

