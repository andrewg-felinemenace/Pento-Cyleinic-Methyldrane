# Pento Cyleinic Methyldrane

## Seriously, what's up with the name?

Youtube link, doctor who!

## About 

Pento Cyleinic Methyldrane (PCM) removes freedom[1] from programs to execute 
arbitrary system calls using the secure computing (seccomp) interface provided
by the Linux Kernel.

As an example, you could use the seccomp interface to prevent programs from 
being able to use all system calls except the open() system call, or to 
disallow all system calls except for the system calls you define.

PCM is a tool to experiment with the seccomp interface, and using it as an 
"after market" tool to either help secure applications, or quickly experiment
with how much 


## Creating sandboxes 

XXX, to do

## Technical notes

In order to remove as many system calls as possible from the target
application, it works by acting as a shared library, and once a suitable
function has been called, it installs the filter.

## Configuration files

XXX, to do

## Examples

### Block /bin/cat from reading files

```
root@pcm:~/pcm# cat /etc/pcm/bin_cat.json 
{
        "default": "ALLOW",
        "hook": "start",
        "rules": [
                {
                        "syscall": "read",
                        "action": "ERRNO" 
                }
        ]
}
root@pcm:~/pcm# LD_PRELOAD=bin/libpcm.so PCM_POLICY_FILE=/etc/pcm/bin_cat.json cat /etc/pcm/bin_cat.json
cat: /etc/pcm/bin_cat.json: Operation not permitted
```

### Breaking shellcode

Consider the logic behind connect back or bind port shellcode

```c
int fd, cfd;
struct sockaddr_in sin;

fd = socket(AF_INET, SOCK_STREAM, 0);
sin.sin_family = AF_INET;
sin.sin_addr.s_addr = 0; // inet_addr("0.0.0.0"); 
sin.sin_port = htons(4444);
bind(fd, &sin, sizeof(sin));
listen(fd, 5);
cfd = accept(fd, NULL, 0);
dup2(cfd, 0);
dup2(cfd, 1);
dup2(cfd, 2);

// perhaps some cleanup of fd and cfd, but generally unlikely
execve("/bin//sh", [ "/bin//sh", NULL ], NULL);
```

Generally speaking, network applications won't use the dup2() system call after 
initialization (as they generally set /dev/null to fd's 0, 1 and 2), so it becomes
feasible to use the seccomp interface to kill the process if that system call occurs. 

We could use the following configuration file after an accept() libc call to kill the
process if it uses the dup2() system call.

```json
{
        "default": "ALLOW",
        "hook": "listen",
        "rules": [
                {
                        "syscall": "dup2",
                        "action": "KILL" 
                }
        ]
}
```

Another approach is to look at the system calls a variety of shellcodes do, and compare
how likely it is to occur in normal system operation.

An example of a generally unlikely system call is the execve(prog, [ prog ], NULL) (with
the char *const envp[] being NULL), so we could prevent execve() succeeding when envp[] is
set to NULL (using ERRNO mechanism, for example).

Currently, PCM doesn't support argument filtering, but this will come at a later date :>

### Preventing kernel exploitation

enlightenment.tar.gz 

old ptrace bug
perf_counter
move_pages

#### Theory

We can help prevent kernel exploitation by restricting access to system calls that 
contain exploitable vulnerabilities (or perhaps, are new, and thus historically likely
to be vulnerable).

In order to do this, we ideally need to restrict every process on the system 
(from init onwards), however, restricting just network facing daemons may suffice
(for example, a web server may restrict httpd and sshd) ..

#### Preventing system calls system wide 

To load the library into the init process, we can use /etc/ld.so.preload and ask
init to re-execute itself (telinit u) ..

#### Verifying that processes are restricted 

```sh

cat /proc/$PID/status | grep Seccomp
Seccomp: 2
```

#### XXX, don't do this section (titled Danger, Will Robinson?)

### Whitelisting system calls and arguments





[1] youtube link 
