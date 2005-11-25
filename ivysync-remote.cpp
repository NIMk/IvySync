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


char *help =
"Usage: ivysync-remote [-s server:port] command [arguments]\n"
"  -h --help      show this help\n"
"  -D --debug     print verbos debugging messages\n"
"  -s --server    server:port address to connect (default localhost:264)\n"
"  -i --inspect   inspect methods available on host\n";

char *short_options = "-hD:s:i";
const struct option long_options[] = {
  { "help", no_argument, NULL, 'h'},
  { "debug", required_argument, NULL, 'D'},
  { "server", required_argument, NULL, 's'},
  { "inspect", no_argument, NULL, 'i'},
  {0, 0, 0, 0}
};


bool introspect = false;
char server[512];
int  port;


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
      introspect = true;
      break;

    case 's':
      c = sscanf(optarg,"%s:%d",server,&port);
      if(c != 2) {
        E("error parsing server option: %s",optarg);
	E("malformed syntax, please specify hostname:port");
	exit(0);
      }
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

  XmlRpcValue params;
  XmlRpcValue result;
  XmlRpc::setVerbosity(5);
  XmlRpcClient c(server, port);

  if(inspect) {
    if( c.execute("system.listMethods", params, result) )
      A("inspecting methods on %s:%i", server, port);
      fprintf(stderr,"%s\n",result);
    } else
      E("error calling system.listMethods on %s:%i", server, port);
    exit(1);
  }

  N("TODO");
  exit(1);

}
