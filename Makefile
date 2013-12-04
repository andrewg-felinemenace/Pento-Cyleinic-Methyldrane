all:
	gcc -c src/pcm_policy.c -o bin/pcm_policy.o -Isrc -O3 -Wall
	gcc -c src/pcm_hooks.c -o bin/pcm_hooks.o -Isrc -O3 -Wall
	gcc -c src/pcm_init.c -o bin/pcm_init.o -Isrc -O3 -Wall
	ar r bin/libpcm.a bin/*.o
	gcc -o bin/00-exit tests/00-exit.c -Lbin -lpcm -Llibseccomp-2.1.1/src/ -lseccomp -ljansson -Isrc 
	gcc -o bin/00-exit-fp tests/00-exit-fp.c -Lbin -lpcm -Llibseccomp-2.1.1/src/ -lseccomp -ljansson -Isrc
	gcc -o bin/01-setuid tests/01-setuid.c -Lbin -lpcm -Llibseccomp-2.1.1/src/ -lseccomp -ljansson -Isrc -ldl
	gcc -o bin/01-setuid-fp tests/01-setuid-fp.c -Lbin -lpcm -Llibseccomp-2.1.1/src/ -lseccomp -ljansson -Isrc -ldl
	gcc -o bin/01-accept tests/01-accept.c -Lbin -lpcm -Llibseccomp-2.1.1/src/ -lseccomp -ljansson -Isrc -ldl
	gcc -o bin/01-accept-fp tests/01-accept-fp.c -Lbin -lpcm -Llibseccomp-2.1.1/src/ -lseccomp -ljansson -Isrc -ldl
