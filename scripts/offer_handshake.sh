#!/bin/bash

# here configure the file with the list of IP or hostnames, one per line
LISTFILE=list

if [ -z $1 ]; then
        echo "usage: $0 network_interface"
        echo "example: $0 eth0"
        exit 1
fi
IFACE="$1"

IP="`ifconfig $IFACE | grep 'inet addr'| awk '{print $2}'|cut -f2 -d:`"

ready=false

for i in `cat $LISTFILE`; do
	rm -f /tmp/handshake.$i.ok
	echo -n "handshaking $i"

	# background listener
	(answer=`echo | nc -q 0 -u -l 3331`;
	 echo $answer > /tmp/handshake.$i.ok) &

	while ! [ -r /tmp/handshake.$i.ok ]; do
		sleep 1
		../udpbroadcast $i 3332 $IP 1>&2 > /dev/null
		echo -n "."
	done	
	echo -n " answer: `cat /tmp/handshake.$i.ok`"
	rm /tmp/handshake.$i.ok
	echo
done 

exit 0
