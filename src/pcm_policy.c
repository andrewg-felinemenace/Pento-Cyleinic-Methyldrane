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


// this is the maximum argument count supported ..
//
// ../src/arch-x32.h:#define x32_arg_count_max             6
// ../src/arch-x86.h:#define x86_arg_count_max             6
// ../src/arch.h:#define ARG_COUNT_MAX     6
// ../src/arch-arm.h:#define arm_arg_count_max             6
// ../src/arch-x86_64.h:#define x86_64_arg_count_max               6

#ifndef ARG_COUNT_MAX
#define ARG_COUNT_MAX 6
#endif

int pcm_json_op_to_define(const char *op)
{
	// As from the documentation, 

	// SCMP_CMP_NE = 1,                /**< not equal */
        // SCMP_CMP_LT = 2,                /**< less than */
        // SCMP_CMP_LE = 3,                /**< less than or equal */
        // SCMP_CMP_EQ = 4,                /**< equal */
        // SCMP_CMP_GE = 5,                /**< greater than or equal */
        // SCMP_CMP_GT = 6,                /**< greater than */
        // SCMP_CMP_MASKED_EQ = 7,         /**< masked equality */

	if(strcasecmp(op, "ne") == 0) return SCMP_CMP_NE;
	if(strcasecmp(op, "lt") == 0) return SCMP_CMP_LT;
	if(strcasecmp(op, "le") == 0) return SCMP_CMP_LE;
	if(strcasecmp(op, "eq") == 0) return SCMP_CMP_EQ;
	if(strcasecmp(op, "ge") == 0) return SCMP_CMP_GE;
	if(strcasecmp(op, "gt") == 0) return SCMP_CMP_GT;
	if(strcasecmp(op, "masked_eq") == 0) return SCMP_CMP_MASKED_EQ;

	errx(EXIT_FAILURE, "json operation value is not handled here - %s", op);

}

void pcm_json_restriction_to_arg_cmp(json_t *restriction, struct scmp_arg_cmp *arg_cmp)
{
	json_t *arg, *op, *datum_a, *datum_b;

	// XXX, test cases for datum_* handling / jansson integer handling etc.

	arg = json_object_get(restriction, "arg");
	op = json_object_get(restriction, "op");
	datum_a = json_object_get(restriction, "datum_a");
	datum_b = json_object_get(restriction, "datum_b");

	if(! json_is_integer(arg)) {
		errx(EXIT_FAILURE, "syscall argument number is not an integer");
	}

	if(! json_is_string(op)) {
		errx(EXIT_FAILURE, "operation value is not a string");
	}

	if(! json_is_integer(datum_a)) {
		errx(EXIT_FAILURE, "datum_a is not an integer");
	}

	if(datum_b) {
		if(! json_is_integer(datum_b)) {
			errx(EXIT_FAILURE, "datum_b is not an integer");
		}
	}

	arg_cmp->arg = json_integer_value(arg);
	arg_cmp->op = pcm_json_op_to_define(json_string_value(op));
	arg_cmp->datum_a = json_integer_value(datum_a);
	arg_cmp->datum_b = datum_b ? json_integer_value(datum_b) : 0;
}

void pcm_json_rule_to_seccomp(json_t *rule)
{
	json_t *syscall, *action, *restrictions;
	int rc;
	int syscall_num;
	int arg_count;
	int i;
	int default_action;
	struct scmp_arg_cmp arg_cmp[ARG_COUNT_MAX];
		
		
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

	arg_count = 0;
	restrictions = json_object_get(rule, "restrictions");

	if(restrictions) {
		if(! json_is_array(restrictions)) {
			errx(EXIT_FAILURE, "restrictions is not an array");
		}

		if(json_array_size(restrictions) > ARG_COUNT_MAX) {
			errx(EXIT_FAILURE, "too many restrictions :/");
		}

		for(i = 0; i < json_array_size(restrictions); i++) {
			json_t *arg_restriction;
			arg_restriction = json_array_get(restrictions, i);
			if(! arg_restriction) {
				errx(EXIT_FAILURE, "fetching entry from array failed?");
			}

			if(! json_is_object(arg_restriction)) {
				errx(EXIT_FAILURE, "argument restriction is not an object");
			}

			pcm_json_restriction_to_arg_cmp(arg_restriction, &arg_cmp[i]);
		}
		arg_count = i;
	}

	rc = seccomp_rule_add_array(PCM_GLOBAL.seccomp, default_action, syscall_num, arg_count, arg_cmp);
	if(rc < 0) {
		errx(EXIT_FAILURE, "Adding rule failed?");
	}


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
		json_t *rule;


		rule = json_array_get(rules, i);
		if(! json_is_object(rule)) {
			errx(EXIT_FAILURE, "Expected rule object, got something else instead");
		}
		
		pcm_json_rule_to_seccomp(rule);
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
