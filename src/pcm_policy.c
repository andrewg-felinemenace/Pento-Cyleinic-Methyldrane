#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>

#include <jansson.h>
#include <seccomp.h>

#include "pcm.h"

int pcm_policy_parse(char *filename, json_error_t *error)
{
	PCM_GLOBAL.policy = json_load_file(filename, 0, error);
	return !PCM_GLOBAL.policy;
	
}

void pcm_policy_free()
{
	if(PCM_GLOBAL.policy) json_decref(PCM_GLOBAL.policy);
	PCM_GLOBAL.policy = NULL;
}

u_int32_t pcm_string_to_policy(const char *str)
{
	if(strcasecmp(str, "ALLOW") == 0) {
		return SCMP_ACT_ALLOW;
	} else if(strcasecmp(str, "KILL") == 0) {
		return SCMP_ACT_KILL;
	} else if(strcasecmp(str, "ERRNO") == 0) {
		return SCMP_ACT_ERRNO(EPERM);
	} 

	return -1;
}

int pcm_json_to_seccomp(char **hook)
{
	json_t *str, *rules, *hook_obj;
	u_int32_t default_action;
	int i, rc;

	if(hook) *hook = NULL; // initialize to a sane value

	/* Verify it's what we expect */
	if(! json_is_object(PCM_GLOBAL.policy)) {
		errx(EXIT_FAILURE, "root is not an object");
	}	

	/* Get the default action */
	str = json_object_get(PCM_GLOBAL.policy, "default");
	if(! json_is_string(str)) {
		errx(EXIT_FAILURE, "expected string, didn't get it :/");
	}

	default_action = pcm_string_to_policy(json_string_value(str));
	if(default_action == -1) {
		errx(EXIT_FAILURE, "converting default policy string failed, expecting ALLOW, KILL, or ERRNO");
	}

	/* Initialize seccomp with the default action */
	PCM_GLOBAL.seccomp = seccomp_init(default_action);
	if(PCM_GLOBAL.seccomp == NULL) {
		errx(EXIT_FAILURE, "Initializing seccomp context failed");
	}

	str = json_object_get(PCM_GLOBAL.policy, "no_new_privs");
	if(str) {
		// for another day, default version of jansson on this system doesn't have it.
		// int status;
		// status = json_boolean(str);
		// we'll assume they want to disable it.

		rc = seccomp_attr_set(PCM_GLOBAL.seccomp, SCMP_FLTATR_CTL_NNP, 0);	
		if(! rc) {
			errx(EXIT_FAILURE, "Unable to set no_new_privs attribute to off (%m) and rc = %d, -rc = %d\n", rc, -rc);
		}
	}

	/* And loop over the rules */

	rules = json_object_get(PCM_GLOBAL.policy, "rules");
	if(! json_is_array(rules)) {
		errx(EXIT_FAILURE, "Getting policy rules from json failed");
	}

	if(hook) {
		// XXX, should the file hook value overwrite env var etc?
		// should we just pcm_hook_set() here?
		hook_obj = json_object_get(PCM_GLOBAL.policy, "hook");
		if(hook_obj) {
			if(! json_is_string(hook_obj)) {
				errx(EXIT_FAILURE, "expected hook string, didn't get it :/");
			}

			*hook = strdup(json_string_value(hook_obj));
		}
	}

	for(i = 0; i < json_array_size(rules); i++) {
		json_t *rule, *syscall, *action;
		int rc;
		int syscall_num;
		
		
		rule = json_array_get(rules, i);
		if(! json_is_object(rule)) {
			errx(EXIT_FAILURE, "Expected rule object, got something else instead");
		}
		
		syscall = json_object_get(rule, "syscall");
		if(! json_is_string(syscall)) {
			errx(EXIT_FAILURE, "Expected string for system call");
		}

		action = json_object_get(rule, "action");
		if(! json_is_string(action)) {
			errx(EXIT_FAILURE, "Expected string for action value");
		}

		default_action = pcm_string_to_policy(json_string_value(action));
		if(default_action == -1) {
			errx(EXIT_FAILURE, "Rule action invalid (expecting ALLOW, KILL, or ERRNO)");
		}	
	
		syscall_num = seccomp_syscall_resolve_name(json_string_value(syscall));
		if(syscall_num == __NR_SCMP_ERROR) {
			errx(EXIT_FAILURE, "System call number is negative?");
		}

		rc = seccomp_rule_add(PCM_GLOBAL.seccomp, default_action, syscall_num, 0);
		if(rc < 0) {
			errx(EXIT_FAILURE, "Adding rule failed?");
		}

	}

	pcm_policy_free();

	return 0;
}

void pcm_install_policy()
{
	int rc;

	rc = seccomp_load(PCM_GLOBAL.seccomp);
	if(rc < 0) {
		errx(EXIT_FAILURE, "Failed to load the seccomp policy into the process");
	}
	seccomp_release(PCM_GLOBAL.seccomp);
	PCM_GLOBAL.seccomp = NULL;
}

int pcm_load_policy_from_file(char *filename, char **hook)
{
	json_error_t error;

	if(pcm_policy_parse(filename, &error)) {
		errx(EXIT_FAILURE, "error on line %d: '%s' from %s", error.line, error.text, filename);
	}

	if(pcm_json_to_seccomp(hook)) {
		errx(EXIT_FAILURE, "failed to convert json to seccomp");
	}

	return 0;
}

int pcm_try_load_policy_from_file(char *filename, char **hook)
{
	json_error_t error;

	if(! pcm_policy_parse(filename, &error)) {
		pcm_json_to_seccomp(hook);
	}
	return !PCM_GLOBAL.seccomp;
}

#if 0
int main(int argc, char **argv, char **envp)
{
	int fd;
	int rc;
	char buf[8];

	pcm_load_policy_from_file("allow_all.json");
	pcm_load_policy_from_file("deny_read.json");
	
	fd = open("/etc/passwd", O_RDONLY);

	printf("going to read from file!\n");

	rc = read(fd, buf, 7);
	if(rc == -1) {
		printf("failed to read: %m");
	}

	return 0;
}
#endif
