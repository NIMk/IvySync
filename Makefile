all:
	# to build run the compile.sh script

clean:
	rm -f *.a *.o *~
	rm -f \
		ivysync-nox \
		ivysync-rpc xmlrpc++/*.o xmlrpc++/*.a \
		ivysync-gtk \
		ivysync-udp udpbroadcast

