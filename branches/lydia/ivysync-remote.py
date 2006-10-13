#!/usr/bin/python

# python xmlrpc client for ivysync
# (C)2006 by Denis "Jaromil" Rojo
# released under the GNU General Public License

# general system functions
import sys
from string import atoi

# xmlrpclib client functions
from socket import gethostname
from xmlrpclib import Transport, dumps





######################################################################
# XMLRPC Client connection class

class xmlrpc_connection:
    """The xmlrpc_connection class tests the xmlrpc_server.  You must 
    download and install the medusa and xmlrpclib libraries to run 
    this code:  http://www.nightmare.com  http://www.pythonware.com"""

    def __init__(self, host=None, port=2640):
        if host is None:
            host = gethostname()
        self.host = "%s:%s" % (host, port)
        self.transport = Transport()

    def remote(self, method, params=()):
        """remote invokes the server with the method name and an 
        optional set of parameters.  The return value is always a
        tuple."""

        response = self.transport.request(self.host, 
                                          '/RPC2',
                                          dumps(params, method))
        return response

###################################################################





###################################################################
## MAIN



if __name__ == '__main__':

    commands = ("Play", "SyncStart", "Stop", "Open", "Pause", "Quit", "GetPos", "SetPos", "GetOffset", "SetOffset", "SyncOffset")
    
    connection = xmlrpc_connection()

#    if(sys.argc):
#    	cmd = "none"
#    else:
    cmd = sys.argv[1]

    
    if not cmd in commands:

        print "no valid command recognized, list of valid commands:"
        print commands
        sys.exit(2)
      
    if   cmd == "SyncStart":
	(res,) = connection.remote(cmd, (0, 0))
	if res == 1:
		answer = "Sync starting OK"
	else:
		answer = "Error in sync starting"

    elif cmd == "SyncOffset":

        pos  = int( sys.argv[2] )

	(res,) = connection.remote(cmd, (pos, 0))
        if res == 1:
		answer = "Global sync to offset " + str(pos)
	else:
		answer = "Error in SyncOffset"

    elif cmd == "Quit":
        
        (res,) = connection.remote(cmd, (0, 0))
        if res == 1:
            answer = "Server is now quitting"
        else:
            answer = "Server refuses to quit"


    elif cmd == "SetPos":

        chan = int( sys.argv[2] )
        pos  = int( sys.argv[3] )

        (res,) = connection.remote(cmd, (chan, pos))
        answer = "Channel " + str(chan) + " is now at " + str(pos) + "%"

    elif cmd == "SetOffset":
	chan = int( sys.argv[2] )
	pos  = int( sys.argv[3] )

	(res,) = connection.remote(cmd, (chan, pos))
	answer = "Channel " + str(chan) + " is now at " + str(pos) + " byte offset"

    elif cmd == "GetPos":

        chan = int( sys.argv[2] )
        (res,) = connection.remote(cmd, (chan, 0))
        answer = "Channel " + str(chan) + " is now at " + str(res) + "%"

    elif cmd == "GetOffset":

        chan = int( sys.argv[2] )
        (res,) = connection.remote(cmd, (chan, 0))
        answer = "Channel " + str(chan) + " is now at " + str(res) + " byte offset"

    elif cmd == "Open":

        chan = int(sys.argv[2] )
        path = str(sys.argv[3] )
        (res,) = connection.remote(cmd, (chan, path))
        if res == 1:
            answer = "File " + path + " opened on channel " + str(chan)
        else:
            answer = "Error opening file " + path
            
    else: # all other commands needing only one argument (channel num)
        
        chan = int( sys.argv[2] )

        (res,) = connection.remote(cmd, (chan, 0))
        if res == 1:
            answer = "Command '" + cmd + "' succesfully executed on channel " + str(chan)
        else:
            answer = "Error executing command '" + cmd + "' on channel " + str(chan)

    print answer # 42! ;D
