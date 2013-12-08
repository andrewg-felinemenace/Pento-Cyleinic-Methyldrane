#!/bin/sh

cd ../bin

for i in *-* ; do 
	echo -n "[*] Testing $i ... "
	LD_LIBRARY_PATH=. ./$i >/dev/null 2>/dev/null
	saved=$?
	shorter=`echo $i | sed -r s/-.?fp//g`
	#echo "\$i is $i, and \$shorter is $shorter"

	if [ "$i" =  "$shorter" ] ; then
		[ $saved -eq 0 ] && echo "passed"
		[ $saved -ne 0 ] && echo "failed"
	else 
		[ $saved -eq 0 ] && echo "failed"
		[ $saved -ne 0 ] && echo "passed"
	fi
done

