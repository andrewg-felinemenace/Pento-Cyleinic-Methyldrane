# Pento Cyleinic Methyldrane

This is work in progress / experimental at the moment, and mostly intended for my own amusement
at the moment. So keep that in mind :)

## Seriously, what's up with the name?

Youtube link, doctor who, the sun makers, 2nd video, in processing centre

## About 

Pento Cyleinic Methyldrane (PCM) removes freedom[1] from programs to execute 
arbitrary system calls using the secure computing (seccomp) interface provided
by the Linux Kernel.

As an example, you could use the seccomp interface to prevent programs from 
being able to use all system calls except the open() system call, or to 
disallow all system calls except for the system calls you define.

PCM is a tool to experiment with the seccomp interface, and using it as an 
"after market" tool to either help secure applications, or quickly experiment
with how much effort is required to add sandboxing to an application.

## Creating sandboxes 

XXX, to do

## Technical notes

In order to run with the least amount of system calls possible, it's best to
install the filter after the application has executed general initialization
functions, then drops privilege / just before it handles untrusted input.

In order to do this, PCM is a shared library which can hook some limited
functions (such as setuid, etc) to indicate it's time to install the filter.

## Configuration files

Configuration files currently use JSON to store the data in, though this is subject
to change once the requirements of the configuration files are set in stone.

The configuration is based as a JSON object, which sets the default action (what
happens when no rules match), when it should install the filter, and what rules are present.

An example of a configuration file that allows all system calls, but makes read return -EPERM
is as follows:

```json
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
```

An example of a default deny, all specific system calls is

```json
{
        "default": "KILL",
        "hook": "start",
        "rules": [
                {
                        "syscall": "exit_group",
                        "action": "ALLOW" 
                }
        ]
}
```

You can even restrict the arguments specified to the system call.
It's possible to test if it's not equal (ne), less than (lt),
lesser than or equal to (le), equal (eq), greater than or equal to (ge),
greater than (gt), and finally, masked equally.

An example of only allowing odd-length read lengths can be implemented by
masked equally.

```
{
        "default": "kill",
        "hook": "start",
        "rules": [
                { 
                        "action": "allow", 
                        "syscall": "read",
                        "restrictions": [
                                { "arg": 2, "op": "masked_eq", "datum_a": 1, "datum_b": 1 }
                        ]
                },
                { "action": "allow", "syscall": "exit_group" },
                { "action": "allow", "syscall": "write" },
                { "action": "allow", "syscall": "exit" }
        ]
}
```

Which checks the 2nd (starting from 0) argument to read(), which performs 

```c
(arg_2 & datum_a) == datum_b
```

and only allows the system call if the result matches.

The masked equal can be used to check arguments which uses bit flags to specify values.

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

(There are some exceptions to this, such as inetd style socket servers

We could use the following configuration file after an accept() libc call to kill the
process if it uses the dup2() system call.

```json
{
        "default": "ALLOW",
        "hook": "accept",
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
set to NULL (using ERRNO mechanism, for example). This type of restriction would most
likely be a suitable system wide restriction.


```json
{
        "default": "ALLOW",
        "hook": "accept",
        "rules": [
                {
                        "syscall": "execve",
                        "action": "KILL",
			"restrictions": [
				{ "arg": 2, "op": "eq", "datum_a": 0 }
			]
                }
        ]
}
```

### Sandboxing lighttpd

#### XXX, you really should use lighttpd's chroot support

A weakness of this approach using the default configuration on Ubuntu is that
lighttpd is not chroot()'d. Due to this configuration, it would be possible for
customized payloads to perform actions such as reading world readable files, etc.

#### XXX, if you want php / cgi restricted, you need to do fastcgi etc servers as well

#### Policy

Recording system calls with strace 

```sh
strace -o /tmp/lighttpd -f /usr/sbin/lighttpd -f /etc/lighttpd/lighttpd.conf
```

After starting lighttpd under strace, use the website as you normally would, 
exercising as much functionality that you use.

After stopping strace, you can examine the system calls recorded. We're interested
in the system calls that occured after privilege dropping (in lighttpd's case, 
it does a setuid(unprivileged uid) which indicates that the privilege dropping is
complete).

By deleting all system calls before and including the setuid() line, we end up with
the used system calls after lighttpd has dropped it's privileges.

Processing the system call recording

```sh
sed s/\(.*//g /tmp/lighttpd | awk '{ print $2 }' | sort | uniq -c | sort -n -r | awk '{ print $2 }' > usr_sbin_lighttpd.json
```

This sorts the system calls made by lighttpd, and outputs the most common system
calls first. We can then add in the rest of the required contents.

Which gives us a policy of


```json
{
        "default": "KILL",
        "hook": "setuid",
        "rules": [
                { "action": "ALLOW", "syscall": "epoll_wait" },
                { "action": "ALLOW", "syscall": "close" },
                { "action": "ALLOW", "syscall": "open" },
                { "action": "ALLOW", "syscall": "fcntl" },
                { "action": "ALLOW", "syscall": "read" },
                { "action": "ALLOW", "syscall": "mkdir" },
                { "action": "ALLOW", "syscall": "rt_sigaction" },
                { "action": "ALLOW", "syscall": "setsockopt" },
                { "action": "ALLOW", "syscall": "ioctl" },
                { "action": "ALLOW", "syscall": "accept" },
                { "action": "ALLOW", "syscall": "stat" },
                { "action": "ALLOW", "syscall": "writev" },
                { "action": "ALLOW", "syscall": "shutdown" },
                { "action": "ALLOW", "syscall": "sendfile" },
                { "action": "ALLOW", "syscall": "brk" },
                { "action": "ALLOW", "syscall": "write" },
                { "action": "ALLOW", "syscall": "munmap" },
                { "action": "ALLOW", "syscall": "mmap" },
                { "action": "ALLOW", "syscall": "fstat" },
                { "action": "ALLOW", "syscall": "exit_group" },
                { "action": "ALLOW", "syscall": "clone" },
                { "action": "ALLOW", "syscall": "setsid" },
                { "action": "ALLOW", "syscall": "lseek" },
                { "action": "ALLOW", "syscall": "getuid" },
                { "action": "ALLOW", "syscall": "getgid" },
                { "action": "ALLOW", "syscall": "epoll_ctl" },
                { "action": "ALLOW", "syscall": "epoll_create" },
                { "action": "ALLOW", "syscall": "chdir" },
		{ "action": "ALLOW", "syscall": "statfs" }
        ]
}
```

As we can see here, we've significantly restricted the system calls available to the
lighttpd process, and if it attempts to execute any other system calls, the process
is killed.

Just filtering on system calls can have weaknesses .. should filter on args ..

XXX, mention customized shellcode that reads /etc/passwd, then tries for weak
permissions on /path/to/home/.dotfiles to insert code that runs without restrictions ..

XXX, defense in depth, ensure chroot configuration is used, and perhaps an ACL interface
by the kernel

### Preventing kernel exploitation

enlightenment.tar.gz 

old ptrace bug
perf_counter
move_pages

XXX, you really should upgrade the kernel and not rely on this. 

#### Theory

We can help prevent kernel exploitation by restricting access to system calls that 
contain exploitable vulnerabilities (or perhaps, are new, and thus historically likely
to be vulnerable).

In order to do this, we ideally need to restrict every process on the system 
(from init onwards), however, restricting just network facing daemons may suffice
(for example, a web server may restrict httpd and sshd) ..

However, for this to work, you must be able to log in as root at the console or
over the network - it seems that the libseccomp2 library doesn't allow you to
disable the no new privs functionality. 

#### Preventing system calls system wide 

(XXX, reboot is best bet .. ?)

To load the library into the init process, we can use /etc/ld.so.preload and ask
init to re-execute itself (telinit u) ..

After init has been restarted, you need to restart affected services. With upstart, 
you can use the initctl command to restart processes, for example:

```sh
initctl restart ssh
```



#### Verifying that processes are restricted 

```sh

cat /proc/$PID/status | grep Seccomp
Seccomp: 2
```

However, I'm not aware of any way of dumping the filters associated with a 
process or thread at the moment. 

#### XXX, don't do this section (titled Danger, Will Robinson?)


[1] youtube link 
