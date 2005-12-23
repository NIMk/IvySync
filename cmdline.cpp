
/*  IvySync - Video SyncStarter
 *
 *  (c) Copyright 2004-2006 Denis Roio aka jaromil <jaromil@dyne.org>
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
 * to compile this sourcecode: gcc -o ivysync ivysync.c -lpthreads
 * it should work on any POSIX system, including embedded hardware
 * wherever the IvyTV drivers can also run (see http://ivtv.sf.net)
 *
 */

#include <cstdio>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>

#include <decoder.h>

#include <xmlrpc.h>
#include <gui.h>

#include <utils.h>


bool syncstart = false;
bool graphical = false;
bool dummytest = false;
bool rpcdaemon = false;
int videobuf = 64;

// our global vector holding all instantiated decoders
vector<Decoder*> decoders;

// graphical interface
Gui *gui;

// xmlrpc interface
XmlRpcServer *xmlrpc;

// Threaded daemon
IvySyncDaemon *daemonthread;

char *help =
"Usage: ivysync [-hsDgt] [ -d /dev/video16 [ -p playmode files ] ]\n"
"  -h --help      show this help\n"
"  -t --test      dummy testrun: don't open devices\n"
"  -D --debug     print verbose debugging messages\n"
"  -s --scan      scan for available devices\n"
"  -d --device    activate a device (i.e. /dev/video16)\n"
"  -b --buffer    size of video buffer in KB (default 64)\n"
"  -p --playmode  playlist mode (play|cont|loop|rand)\n"
"  -x --xmlrpc    run XmlRpc daemon for remote control\n"
"  -g --gui       start the graphical user interface\n";

char *short_options = "-hd:sb:xp:gtD:";
const struct option long_options[] = {
  { "help", no_argument, NULL, 'h'},
  { "device", required_argument, NULL, 'd'},
  { "buffer", required_argument, NULL, 'b'},
  { "scan", no_argument, NULL, 's'},
  { "xmlrpc", no_argument, NULL, 'x'},
  { "playmode", required_argument, NULL, 'p'},
  { "gui", no_argument, NULL, 'g'},
  { "test", no_argument, NULL, 't'},
  { "debug", required_argument, NULL, 'D'},
  {0, 0, 0, 0}
};


void quitproc (int Sig) { /* signal handling */
  N("received signal %u on process %u",Sig,getpid());  
  A("please wait while quitting threads");
  vector<Decoder*>::iterator dec_iter;

  if(graphical) gtk_main_quit();

  for( dec_iter = decoders.begin();
       dec_iter != decoders.end();
       ++dec_iter)
    (*dec_iter)->close();

}

#define CHECK_DECODER \
	if(!dec) { \
          dec = new Decoder(); \
	  if( dec->init("/dev/video16") ) { \
	    decoders.push_back( dec ); \
	  } else { \
	    delete dec; \
	    dec = NULL; \
	  } \
        }

int cmdline(int argc, char **argv) {
  Decoder *dec = NULL;
  FILE *fd;
  int c;
  int res;

  N("IvySync 0.2 / (c)2004-2006 Denis Rojo <jaromil@dyne.org>");

  do { 
    res = getopt_long(argc, argv, short_options, long_options, NULL);
    
    switch(res) {
      
    case 'h':
      fprintf(stderr,"%s",help);
      exit(1);
      break;

    case 'd':
      dec = new Decoder();
      dec->dummy = dummytest;
      if( dec->init(optarg) )
	decoders.push_back( dec );
      else {
	E("can't initialize device %s",optarg);
	delete dec;
	dec = NULL;
      }
      break;

    case 's':
      N("Scanning for available playback devices...");
      dec = new Decoder();
      c=0;

      if( dec->init("/dev/video16") ) {
	A("1. /dev/video16 is present");
	c++;
      }
      dec->close();


      if( dec->init("/dev/video17") ) {
	A("2. /dev/video17 is present");
	c++;
      }
      dec->close();


      if( dec->init("/dev/video18") ) {
	A("3. /dev/video18 is present");
	c++;
      }
      dec->close();


      if( dec->init("/dev/video19") ) {
	A("4. /dev/video19");
	c++;
      }
      dec->close();

      delete dec;
      N("Total of %u device(s) found",c);
      exit(1);
      break;

    case 'p':
      CHECK_DECODER;
      if( strncasecmp(optarg,"play",4) ==0 )
	dec->playmode = PLAY;
      else if( strncasecmp(optarg,"cont",4) ==0 )
	dec->playmode = CONT;
      else if( strncasecmp(optarg,"loop",4) ==0 )
	dec->playmode = LOOP;
      else if( strncasecmp(optarg,"rand",4) ==0 )
	dec->playmode = RAND;
      else
	E("unrecognized playmode: %s",optarg);
      
      break;

    case 'x':
      rpcdaemon = true;
      break;

    case 'g':
      graphical = true;
      break;
      
    case 't':
      dummytest = true;
      break;

    case 'D':
      set_debug( atoi(optarg) );
      break;

    case 'b':
      videobuf = atoi(optarg);
      break;

    case 1:
      fd = fopen(optarg,"rb");
      if(fd) {
	CHECK_DECODER;
	dec->append( optarg );
	fclose(fd);
      } else E("file %s is not readable",optarg);
      
      break;
      
    default: break;
      
    }
    
  } while(res > 0);
  
  return res;
}


int main(int argc, char **argv) {
  vector<Decoder*>::iterator dec_iter;
  Decoder *dec;

  set_debug(1);

  /* register quit signal handlers */
  if (signal (SIGINT, quitproc) == SIG_ERR) {
    fprintf(stderr,"Couldn't install SIGINT handler\n");
    exit (0);
  }
  if (signal (SIGQUIT, quitproc) == SIG_ERR) {
    fprintf(stderr,"Couldn't install SIGQUIT handler\n");
    exit (0);
  }
  if (signal (SIGTERM, quitproc) == SIG_ERR) {
    fprintf(stderr,"Couldn't install SIGTERM handler\n");
    exit (0);
  }


  cmdline(argc, argv);
 
  if( !decoders.size() ) {
  	E("no decoder device is initialized, aborting operations");
	exit(0);
  }

  /////////////////////////////////
  // setup the graphical interface
  if(graphical)
    if(!getenv("DISPLAY")) {
      graphical = false;
      E("can't use graphical interface: no display found");
    }

  if(graphical) {
    //  gtk_set_locale();
    gtk_init(&argc,&argv);
    //  add_pixmap_directory(PACKAGE_DATA_DIR "/" PACKAGE "/pixmaps");  

    gui = new Gui();
    gui->init(&decoders);
    gui->start();

    exit(1);
  }
  ////////////////////////////////


  ////////////////////////////////
  /// setup the XMLRPC interface
  if(rpcdaemon) {
    xmlrpc = new XmlRpcServer();
    // instantiate all classes
    new Play  (xmlrpc, &decoders);
    new Stop  (xmlrpc, &decoders);
    new GetPos(xmlrpc, &decoders);
    new SetPos(xmlrpc, &decoders);

    // instantiate and launch the threaded daemon
    daemonthread = new IvySyncDaemon(xmlrpc);
    daemonthread->launch();

  }

  ////////////////////////////////



  ////////////////////////////////
  /// Syncstart!
  for( dec_iter = decoders.begin();
       dec_iter != decoders.end();
       ++dec_iter) {

    dec = *dec_iter;
    dec->setup( &syncstart, videobuf );
    dec->launch();
    dec->play();

  }
  
  N("Syncing %i players...",decoders.size());
	
  jsleep(0,500);
  A("Start!");
  syncstart = 1;
  ////////////////////////////////


  int still_running = decoders.size();
  
  //  if(daemonize) while(true); // loop infinitely
  //  else

  while(still_running) {
    still_running = 0;
    for( dec_iter = decoders.begin();
	 dec_iter != decoders.end();
	 ++dec_iter) {
      
      dec = *dec_iter;
      if(dec->playing) still_running++;
      jsleep(1,0); // 1 second delay check
    }
  }

  N("quit!");

  exit(1);
}
