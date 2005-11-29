/*  IvySync - Video SyncStarter
 *
 *  (c) Copyright 2004-2005 Denis Roio aka jaromil <jaromil@dyne.org>
 *                          Nederlands Instituut voor Mediakunst
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published 
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <cstdio>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <getopt.h>

#include <XmlRpc.h>

#include <utils.h>

using namespace XmlRpc;
using namespace std;

char *help =
"Usage: ivysync-remote [-s server:port] command [arguments]\n"
"  -h --help      show this help\n"
"  -D --debug     print verbos debugging messages\n"
"  -s --server    server address to connect (default localhost)\n"
"  -p --port      server port to connect (default 264)\n"
"  -i --inspect   inspect methods available on host\n";

char *short_options = "-hD:s:p:i";
const struct option long_options[] = {
  { "help", no_argument, NULL, 'h'},
  { "debug", required_argument, NULL, 'D'},
  { "server", required_argument, NULL, 's'},
  { "port", required_argument, NULL, 'p'},
  { "inspect", no_argument, NULL, 'i'},
  {0, 0, 0, 0}
};


bool inspect = false;
char server[512];
int  port;
char uri[256];

int cmdline(int argc, char **argv) {
  
  int res;
  int c;
  
  N("IvySync 0.3 / (c)2004-2005 Denis Rojo <jaromil@dyne.org>");
  
  strcpy(server,"localhost");
  port = 264;
  
  do {
    res = getopt_long(argc, argv, short_options, long_options, NULL);
    
    switch(res) {
      
    case 'h':
      fprintf(stderr,"%s",help);
      exit(1);
      break;
      
    case 'i':
      inspect = true;
      break;
      
    case 's':
      c = sscanf(optarg,"%s",server);
      if(c != 1) {
        E("error parsing server option: %s",optarg);
	E("malformed syntax, please specify hostname");
	exit(0);
      }
      break;
      
    case 'p':
      port = atoi(optarg);
      break;

    case 'D':
      set_debug( atoi(optarg) );
      break;
      
    default: break;
      
    }
    
  } while(res > 0);

  return res;
}


int main(int argc, char* argv[]) {

  set_debug(1);
  
  cmdline(argc, argv);

  N("connecting to %s:%u",server,port);
  
  XmlRpcValue params;
  XmlRpcValue result;
  //  XmlRpc::setVerbosity(5);

  sprintf(uri,"/RPC2");
  XmlRpcClient c((const char*)server, (int)port, (const char*)uri, true );
  
  if(inspect) {

    A("inspecting methods on %s:%i", server, port);
    c.execute("system.listMethods", params, result);

    if( c.isFault() ) {
      E("error calling system.listMethods on %s:%i", server, port);
      exit(1);
    }

    cout << "\nMethods:\n" << result << "\n\n";

  }
  
  N("TODO");
  exit(1);
  
}
