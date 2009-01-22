#!/bin/bash

if [ -z $1 ]; then
	echo "usage: $0 network_interface"
	echo "example: $0 eth0"
	exit 1
fi
IFACE="$1"


NC="netcat -c"
# check if it is openbsd netcat
NC_ver="`netcat -h 2>&1|head -n 1 | awk '{print $1}'`"
if [ "$NC_ver" = "OpenBSD" ]; then
	echo "using OpenBSD version of netcat"
	NC="netcat -q 0"
fi

# some more version might be around that is not supported..

IP="`ifconfig $IFACE | grep 'inet addr'| awk '{print $2}'|cut -f2 -d:`"

echo "listening on $IFACE configured with address $IP ..."
master="`echo | $NC -u -l 3332`"

echo "contacted by master $master"
echo "$IP" | netcat -u $master 3331

echo "master replied"


exit 0
