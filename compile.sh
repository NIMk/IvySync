#!/bin/sh

echo
echo

echo "IvySync consists of different applications to manipulate and play"
echo "syncstarted videos on multiple channels. First make sure all your"
echo "dependencies are in place (-devel packages on your distribution)"
echo "then choose below what to compile:"

echo

echo "1 - Standalone player - playback configured playlists"
echo "2 - Playlist editor - graphical interface for playlists"
echo "3 - XMLRPC daemon - remote controlled player daemon"
echo "4 - ALL of the above"

echo

echo "press a number and then [enter]"
read -s sel


case $sel in
    1)
	make -f Makefile.player
	;;
    2)
	make -f Makefile.playlist
	;;
    3)
	make -f Makefile.xmlrpc
	;;
    4)
	make -f Makefile.player
	make -f Makefile.playlist
	make -f Makefile.xmlrpc
	;;
    *)
	echo "invalid selection."
	;;
esac




