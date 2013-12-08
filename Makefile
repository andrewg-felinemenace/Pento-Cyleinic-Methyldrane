all:
	gcc -ggdb -fPIC -c src/pcm_policy.c -o bin/pcm_policy.o -Isrc -O3 -Wall
	gcc -ggdb -fPIC -c src/pcm_hooks.c -o bin/pcm_hooks.o -Isrc -O3 -Wall
	gcc -ggdb -fPIC -c src/pcm_init.c -o bin/pcm_init.o -Isrc -O3 -Wall
	ar r bin/libpcm.a bin/*.o
	gcc -ggdb -shared -fPIC -o bin/libpcm.so bin/*.o -ljansson -ldl -Llibseccomp-2.1.1/src/ -lseccomp -Isrc
	gcc -ggdb -o bin/00-exit tests/00-exit.c -Lbin -lpcm -Llibseccomp-2.1.1/src/ -lseccomp -ljansson -Isrc 
	gcc -ggdb -o bin/00-sleep tests/00-sleep.c -Lbin -lpcm -Llibseccomp-2.1.1/src/ -lseccomp -ljansson -Isrc 
	gcc -ggdb -o bin/00-exit-fp tests/00-exit-fp.c -Lbin -lpcm -Llibseccomp-2.1.1/src/ -lseccomp -ljansson -Isrc
	gcc -ggdb -o bin/01-setuid tests/01-setuid.c -Lbin -lpcm -Llibseccomp-2.1.1/src/ -lseccomp -ljansson -Isrc -ldl
	gcc -ggdb -o bin/01-setuid-fp tests/01-setuid-fp.c -Lbin -lpcm -Llibseccomp-2.1.1/src/ -lseccomp -ljansson -Isrc -ldl
	gcc -ggdb -o bin/01-accept tests/01-accept.c -Lbin -lpcm -Llibseccomp-2.1.1/src/ -lseccomp -ljansson -Isrc -ldl
	gcc -ggdb -o bin/01-accept-fp tests/01-accept-fp.c -Lbin -lpcm -Llibseccomp-2.1.1/src/ -lseccomp -ljansson -Isrc -ldl
	gcc -ggdb -o bin/02-readfd tests/02-readfd.c -Lbin -lpcm -Llibseccomp-2.1.1/src/ -lseccomp -ljansson -Isrc -ldl
	gcc -ggdb -o bin/02-readfd-0fp tests/02-readfd-0fp.c -Lbin -lpcm -Llibseccomp-2.1.1/src/ -lseccomp -ljansson -Isrc -ldl
	gcc -ggdb -o bin/02-readfd-1fp tests/02-readfd-1fp.c -Lbin -lpcm -Llibseccomp-2.1.1/src/ -lseccomp -ljansson -Isrc -ldl
	gcc -ggdb -o bin/02-test-ne tests/02-test-ne.c -Lbin -lpcm -Llibseccomp-2.1.1/src/ -lseccomp -ljansson -Isrc -ldl
	gcc -ggdb -o bin/02-test-lt tests/02-test-lt.c -Lbin -lpcm -Llibseccomp-2.1.1/src/ -lseccomp -ljansson -Isrc -ldl
	gcc -ggdb -o bin/02-test-le tests/02-test-le.c -Lbin -lpcm -Llibseccomp-2.1.1/src/ -lseccomp -ljansson -Isrc -ldl
	gcc -ggdb -o bin/02-test-eq tests/02-test-eq.c -Lbin -lpcm -Llibseccomp-2.1.1/src/ -lseccomp -ljansson -Isrc -ldl
	gcc -ggdb -o bin/02-test-ge tests/02-test-ge.c -Lbin -lpcm -Llibseccomp-2.1.1/src/ -lseccomp -ljansson -Isrc -ldl
	gcc -ggdb -o bin/02-test-gt tests/02-test-gt.c -Lbin -lpcm -Llibseccomp-2.1.1/src/ -lseccomp -ljansson -Isrc -ldl
	gcc -ggdb -o bin/02-test-masked-eq tests/02-test-masked-eq.c -Lbin -lpcm -Llibseccomp-2.1.1/src/ -lseccomp -ljansson -Isrc -ldl
	gcc -ggdb -o bin/02-test-masked-eq-fp tests/02-test-masked-eq-fp.c -Lbin -lpcm -Llibseccomp-2.1.1/src/ -lseccomp -ljansson -Isrc -ldl
