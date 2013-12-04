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

...

## Examples



[1] youtube link 
