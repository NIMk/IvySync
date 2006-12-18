
/*  IvySync - Video SyncStarter
 *
 *  (c) Copyright 2004-2006 Denis Rojo aka jaromil <jaromil@dyne.org>
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



#include <decoder.h>

#include <sndfile_decoder.h>

#include <sound_device.h>
#include <xmlrpc.h>
#include <gui.h>
#include <parport.h>

#include <utils.h>


bool useaudio = true;
bool syncstart = false;
bool graphical = false;
bool dummytest = false;
bool rpcdaemon = false;
int rpcdaemonport = 2640;
int videobuf = 64;

// our global linklist holding all instantiated decoders
Linklist decoders;

// graphical interface
Gui *gui = NULL;

// xmlrpc interface
XmlRpcServer *xmlrpc = NULL;

// sound device
SoundDevice *snddev = NULL;

// sound file
MuseDecSndFile *sndfile = NULL;

// Threaded daemon
IvySyncDaemon *ivydaemon = NULL;

// Parallel port controller
ParPort *parport = NULL;


// hardcoded movie offset positions
class Offset {
public:
  Offset(int f, int b, int e) {
    fogstart = f;
    black = b;
    enddrama = e;
  }
  ~Offset() { };
  int fogstart;
  int black;
  int enddrama;
};
Offset *minidrama[5];


char *help =
"Usage: ivysync [-hsDgt] [ -d /dev/video16 [ -p playmode files ] ]\n"
"  -h --help      show this help\n"
"  -t --test      dummy testrun: don't open devices\n"
"  -D --debug     print verbose debugging messages\n"
"  -s --scan      scan for available devices\n"
"  -d --device    activate a device (i.e. /dev/video16)\n"
"  -b --buffer    size of video buffer in KB (default 64)\n"
"  -p --playmode  playlist mode (play|cont|loop|rand)\n"
"  -x --xmlrpc    run XmlRpc daemon on a network port\n"
"  -g --gui       start the graphical user interface\n"
"  -a --noaudio   don't open the sound device\n";

char *short_options = "-hd:sb:x:p:gtD:a";
const struct option long_options[] = {
{ "help", no_argument, NULL, 'h'},
{ "device", required_argument, NULL, 'd'},
{ "scan", no_argument, NULL, 's'},
{ "buffer", required_argument, NULL, 'b'},
{ "xmlrpc", required_argument, NULL, 'x'},
{ "playmode", required_argument, NULL, 'p'},
{ "gui", no_argument, NULL, 'g'},
{ "test", no_argument, NULL, 't'},
{ "debug", required_argument, NULL, 'D'},
{ "noaudio", required_argument, NULL, 'a'},
{0, 0, 0, 0}
};


void quitproc (int Sig) { /* signal handling */
  N("received signal %u on process %u",Sig,getpid());  
  A("please wait while quitting threads");

  if(graphical) gtk_main_quit();

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
  
  N("IvySync 0.3 / (c)2004-2006 Denis Rojo <jaromil@dyne.org>");
  
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

    case 'a':
      useaudio = false;
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
    new SyncStart (xmlrpc, &decoders);
    new SyncOffset(xmlrpc, &decoders);

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

  ////////////////////////////////
  /// Sound device
  if(useaudio) {
    snddev = new SoundDevice();
    // open only for playback
    snddev->open(false, true);
    sndfile = new MuseDecSndFile();
    sndfile->load("/mnt/hd1/1/Lydia/pleistersound_def.wav");
    sndfile->play_once(snddev);
  }
  ////////////////////////////////

  /////////////////////////////////
  // Parallel port
  parport = new ParPort();
  parport->init();
  // flash light at startup
  parport->light(true);
  jsleep(1,0);
  parport->light(false);
  parport->launch();

  /////////////////////////////////
  // video files minidrama offsets
  minidrama[0] = new Offset(1750, 1950, 3200);
  minidrama[1] = new Offset(4680, 4880, 5560);
  minidrama[2] = new Offset(6955, 7140, 7900);
  minidrama[3] = new Offset(9460, 9610, 9990);
  minidrama[4] = new Offset(10380, 10520, 11170);


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

  //////////////////////////////////////
  // live interactive part while playing
  int c, off;
  bool trigger;
  int interval = 100;
  static bool syncer;
  
  for( c=0; c<5; c++) { // cycle thru 5 minidramas
    trigger = false;
    dec = (Decoder*)decoders.begin();
  
    do {
      jsleep(0, interval);
      off = dec->getoffset();
    } while(off < minidrama[c]->fogstart);

    A("fogstart of minidrama %u",c);

    // fog start: reset button and play sound
    parport->button_reset();
    parport->light(true);
    if(useaudio) sndfile->replay = true;

    do { 

      jsleep(0, interval);

      if( parport->button_is_pressed() ) {

        parport->light(false);

        D("seek to offset %u", minidrama[c]->black);
        /// global seek of all channels
        dec = (Decoder*) decoders.begin();
        syncer = false;
        while(dec) {

	  dec->stop();
	  dec->setup(&syncer, 0);
	  dec->setoffset( minidrama[c]->black );
	  dec->play();
	  dec = (Decoder*) dec->next;

        }

        jsleep(0, interval);
        syncer = true;
        break;

      }

      off = dec->getoffset();

    } while(off < minidrama[c]->black);

    parport->light(false);

    // fog end: check button and skip if not pressed
    if(! parport->button_is_pressed() ) {
      A("no button pressed: skipping minidrama %u", c);
      D("seek to offset %u", minidrama[c]->enddrama);
      /// global seek of all channels
      dec = (Decoder*) decoders.begin();
      syncer = false;
      while(dec) {
	dec->stop();
	dec->setup(&syncer, 0);
	dec->setoffset( minidrama[c]->enddrama );
	dec->play();
	dec = (Decoder*) dec->next;
      }
      jsleep(0, interval);
      syncer = true;

    } else {
      A("button was pressed: showing minidrama %u",c);
    }    

    parport->button_reset();

  }
  
  //////////////////////////////////////
  
  if(rpcdaemon) {
    
    // run as a daemon: quit only when requested
    while( ! ivydaemon->quit ) {
      ivydaemon->run(1.0);
      jsleep(0,10);
    }
    
  } else {
    
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
  }
  
  N("quit!");
  

  exit(1);
}
