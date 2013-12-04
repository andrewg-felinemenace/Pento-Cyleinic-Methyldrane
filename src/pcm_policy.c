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

json_t *pcm_policy_parse(char *filename)
{
	json_error_t error;
	json_t *json;

	json = json_load_file(filename, 0, &error);
	if(! json) {
		errx(EXIT_FAILURE, "error on line %d: '%s' from %s", error.line, error.text, filename);
	}

	return json;
}

u_int32_t pcm_string_to_policy(char *str)
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

scmp_filter_ctx *pcm_json_to_seccomp(json_t *policy)
{
	json_t *str, *rules;
	u_int32_t default_action;
	scmp_filter_ctx seccomp;
	int i;

	/* Verify it's what we expect */
	if(! json_is_object(policy)) {
		errx(EXIT_FAILURE, "root is not an object");
	}	

	/* Get the default action */
	str = json_object_get(policy, "default");
	if(! json_is_string(str)) {
		errx(EXIT_FAILURE, "expected string, didn't get it :/");
	}

	default_action = pcm_string_to_policy(json_string_value(str));
	if(default_action == -1) {
		errx(EXIT_FAILURE, "converting default policy string failed, expecting ALLOW, KILL, or ERRNO");
	}

	/* Initialize seccomp with the default action */
	seccomp = seccomp_init(default_action);
	if(seccomp == NULL) {
		errx(EXIT_FAILURE, "Initializing seccomp context failed");
	}

	/* And loop over the rules */

	rules = json_object_get(policy, "rules");
	if(! json_is_array(rules)) {
		errx(EXIT_FAILURE, "Getting policy rules from json failed");
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
		if(syscall_num < 0) {
			errx(EXIT_FAILURE, "System call number is negative?");
		}

		rc = seccomp_rule_add(seccomp, default_action, syscall_num, 0);
		if(rc < 0) {
			errx(EXIT_FAILURE, "Adding rule failed?");
		}

	}

	// and free!
	json_decref(policy);

	return seccomp;
}

void pcm_install_policy(scmp_filter_ctx *seccomp)
{
	int rc;

	rc = seccomp_load(seccomp);
	if(rc < 0) {
		errx(EXIT_FAILURE, "Failed to load the seccomp policy into the process");
	}
	seccomp_release(seccomp);
}

void pcm_load_policy_from_file(char *filename)
{
	json_t *policy;
	scmp_filter_ctx seccomp;

	policy = pcm_policy_parse(filename);
	seccomp = pcm_json_to_seccomp(policy);

	pcm_install_policy(seccomp);
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
