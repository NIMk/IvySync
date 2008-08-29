
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
#include <errno.h>
#include <signal.h>
#include <string.h>

#include <decoder.h>

#ifdef WITH_XMLRPC
#include <xmlrpc.h>
#endif

#ifdef WITH_GUI
#include <gui.h>
#endif


#include <utils.h>


bool syncstart = false;
bool graphical = false;
bool dummytest = false;
bool rpcdaemon = false;
int rpcdaemonport = 2640;
int videobuf = 64;

// our global linklist holding all instantiated decoders
Linklist decoders;

#ifdef WITH_GUI
// graphical interface
Gui *gui;
#endif

#ifdef WITH_XMLRPC
// xmlrpc interface
XmlRpcServer *xmlrpc;
// Threaded daemon
IvySyncDaemon *ivydaemon;
#endif

const char *help =
"Usage: ivysync [-hsDgt] [ -d /dev/video16 [ -p playmode files ] ]\n"
"  -h --help      show this help\n"
"  -t --test      dummy testrun: don't open devices\n"
"  -D --debug     print verbose debugging messages\n"
"  -s --scan      scan for available devices\n"
"  -d --device    activate a device (i.e. /dev/video16)\n"
"  -b --buffer    size of video buffer in KB (default 64)\n"
"  -p --playmode  playlist mode (play|cont|loop|rand)\n"
#ifdef WITH_GUI
"  -g --gui       start the graphical user interface\n"
#endif
"  -x --xmlrpc    run XmlRpc daemon on a network port\n";

const char *short_options = "-hd:sb:x:p:gtD:";

const struct option long_options[] = {
  { "help", no_argument, NULL, 'h'},
  { "device", required_argument, NULL, 'd'},
  { "scan", no_argument, NULL, 's'},
  { "buffer", required_argument, NULL, 'b'},
#ifdef WITH_XMLRPC
  { "xmlrpc", required_argument, NULL, 'x'},
#endif
  { "playmode", required_argument, NULL, 'p'},
#ifdef WITH_GUI
  { "gui", no_argument, NULL, 'g'},
#endif
  { "test", no_argument, NULL, 't'},
  { "debug", required_argument, NULL, 'D'},
  {0, 0, 0, 0}
};


void quitproc (int Sig) { /* signal handling */
  N("received signal %u on process %u",Sig,getpid());  
  A("please wait while quitting threads");

#ifdef WITH_GUI
  if(graphical) gtk_main_quit();
#endif

  Decoder *dec;
  dec = (Decoder*)decoders.begin();
  while(dec) {
    dec->close();
    delete dec;
    dec = (Decoder*)decoders.begin();
  }

}

#define CHECK_DECODER                  \
if(!dec) {                             \
  dec = (Decoder*)decoders[1];         \
  if(!dec) {                           \
    dec = new Decoder();               \
    if( dec->init("/dev/video16") ) {  \
      decoders.append( dec );          \
    } else {                           \
      delete dec;                      \
      dec = NULL;                      \
    }                                  \
  }                                    \
}

int cmdline(int argc, char **argv) {
  Decoder *dec = NULL;
  FILE *fd = NULL;
  int c;
  int res;

  N("IvySync 0.5 / (c)2004-2008 Denis Roio <jaromil@dyne.org>");

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
	decoders.append( dec );
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
      if( strncasecmp(optarg,"play",4) ==0 ) {

	dec->playmode = PLAY;

      } else if( strncasecmp(optarg,"cont",4) ==0 ) {

	dec->playmode = CONT;

      } else if( strncasecmp(optarg,"loop",4) ==0 ) {

	dec->playmode = LOOP;

      } else if( strncasecmp(optarg,"rand",4) ==0 ) {

	dec->playmode = RAND;

      } else if( strncasecmp(optarg,"single",6) ==0 ) {
	
	dec->playmode = SINGLE;

      } else
	E("unrecognized playmode: %s",optarg);
      
      break;

    case 'x':
      rpcdaemon = true;
      rpcdaemonport = atoi(optarg);
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
      if(!fd) {

	E("file %s is not readable: %s",optarg, strerror(errno));

      } else {

	CHECK_DECODER;
	dec->append( optarg );
	fclose(fd);
      }
      
      break;
      
    default: break;
      
    }
    
  } while(res > 0);
  
  return res;
}


int main(int argc, char **argv) {
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
 
  if( !decoders.len() ) {
  	E("no decoder device is initialized, aborting operations");
	exit(0);
  }

#ifdef WITH_GUI

  /////////////////////////////////
  // setup the graphical interface
  if(graphical)
    if(!getenv("DISPLAY")) {
      graphical = false;
      E("can't use graphical interface: no display found");
    }

  if(graphical) {
    A("activating graphical user interface for playlist configuration");
    //  gtk_set_locale();
    gtk_init(&argc,&argv);
    //  add_pixmap_directory(PACKAGE_DATA_DIR "/" PACKAGE "/pixmaps");  

    gui = new Gui();
    gui->init(&decoders);
    gui->start();

    exit(1);
  }
  ////////////////////////////////

#endif

#ifdef WITH_XMLRPC
  ////////////////////////////////
  /// setup the XMLRPC interface
  if(rpcdaemon) {
    A("activating XMLRPC daemon listener for remote network control");

    xmlrpc = new XmlRpcServer();

    // instantiate all classes
    new Play  (xmlrpc, &decoders);
    new Stop  (xmlrpc, &decoders);
    new Pause (xmlrpc, &decoders);
    new GetPos(xmlrpc, &decoders);
    new SetPos(xmlrpc, &decoders);
    new GetOffset(xmlrpc, &decoders);
    new SetOffset(xmlrpc, &decoders);
    new Open  (xmlrpc, &decoders);
    new Quit  (xmlrpc, &decoders);

    ivydaemon = new IvySyncDaemon(xmlrpc);

    if( ! ivydaemon->init( rpcdaemonport) ) {
      E("can't initialize daemon listening");
      delete ivydaemon;
      delete xmlrpc;
      rpcdaemon = false;
    } else
      A("XMLRPC daemon listening for commands on port %u",
	rpcdaemonport);
  }
  ////////////////////////////////
#endif


  ////////////////////////////////
  /// Syncstart!
  
  N("Proceeding to syncstart");
  dec = (Decoder*)decoders.begin();
  while(dec) {

    dec->setup( &syncstart, videobuf );

    dec->launch();

    if( ! rpcdaemon ) {
      // try to load the playlist
      dec->load();      
      dec->play();
    }

    dec = (Decoder*)dec->next;
  }

  if( ! rpcdaemon ) {  
    A("Syncing %i players...",decoders.len());
    
    jsleep(0,500);
    A("Start!");
  }

  syncstart = 1;
  ////////////////////////////////



  
#ifdef WITH_XMLRPC
  if(rpcdaemon) {

    // run as a daemon: quit only when requested
    while( ! ivydaemon->quit ) {
      ivydaemon->run(1.0);
      jsleep(0,10);
    }

    N("quit!");
    exit(1);
  }
#endif
  
  
  // run until all the channels are at the end
  int still_running = decoders.len();
  
  while(still_running) {
    
    still_running = 0;
    dec = (Decoder*)decoders.begin();
    
    while(dec) {
      
      if(dec->playing) still_running++;
      
      jsleep(1,0); // 1 second delay check
      
      dec = (Decoder*)dec->next;
    }
  }

  N("quit!");

  exit(1);
}
