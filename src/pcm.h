
// src/pcm_init.c

struct pcm_global {
        char *policy_file;
        char *hook;
};

extern struct pcm_global PCM_GLOBAL;
// src/pcm_policy.c

void pcm_load_policy_from_file(char *filename);
