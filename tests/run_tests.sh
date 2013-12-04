#!/bin/sh

cd ../bin

for i in *-* ; do 
	echo -n "[*] Testing $i ... "
	./$i >/dev/null 2>/dev/null
	shorter=${i%-fp}

	if [ "$i" =  "$shorter" ] ; then
		[ $? -eq 0 ] && echo "passed"
		[ $? -ne 0 ] && echo "failed"
	else 
		[ $? -eq 0 ] && echo "failed"
		[ $? -ne 0 ] && echo "passed"
	fi
done

