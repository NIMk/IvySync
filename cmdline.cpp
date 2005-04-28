
/*  IvySync - Video SyncStarter
 *
 *  (c) Copyright 2004 Denis Roio aka jaromil <jaromil@dyne.org>
 *                     Nederlands Instituut voor Mediakunst
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
#include <unistd.h>
#include <getopt.h>
#include <signal.h>

#include <decoder.h>
#include <utils.h>

bool syncstart = false;
bool daemonize = false;

// our global vector holding all instantiated decoders
vector<Decoder*> decoders;

char *short_options = "-hd:sDp:";
const struct option long_options[] = {
  { "help", no_argument, NULL, 'h'},
  { "device", required_argument, NULL, 'd'},
  { "scan", no_argument, NULL, 's'},
  { "daemon", no_argument, NULL, 'D'},
  { "playmode", required_argument, NULL, 'p'},
  {0, 0, 0, 0}
};


void quitproc (int Sig) { /* signal handling */
  N("received signal %u on process %u",Sig,getpid());  
  A("please wait while quitting threads");
  vector<Decoder*>::iterator dec_iter;
  
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
  int res;

  do { 
    res = getopt_long(argc, argv, short_options, long_options, NULL);
    
    switch(res) {
      
    case 'd':
      dec = new Decoder();
      if( dec->init(optarg) )
	decoders.push_back( dec );
      else {
	E("can't initialize device %s",optarg);
	delete dec;
	dec = NULL;
      }
      break;

    case 's':
      N("Scanning for available playback devices:");
      dec = new Decoder();

      N("1. /dev/video16");
      if( dec->init("/dev/video16") )
	A("device is present");
      dec->close();

      N("2. /dev/video17");
      if( dec->init("/dev/video17") )
	A("device is present");
      dec->close();

      N("3. /dev/video18");
      if( dec->init("/dev/video18") )
	A("device is present");
      dec->close();

      N("4. /dev/video19");      
      if( dec->init("/dev/video19") )
	A("device is present");
      dec->close();

      delete dec;
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

    case 'D':
      daemonize = true;
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

  set_debug(3);

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
 

  for( dec_iter = decoders.begin();
       dec_iter != decoders.end();
       ++dec_iter) {

    dec = *dec_iter;
    dec->syncstart = &syncstart;
    dec->launch();
    dec->play();

  }
  
  N("%i players to sync",decoders.size());
	
  
  fprintf(stderr,"Waiting 1 second before startup...");
  jsleep(1,0);
  fprintf(stderr," GO!\n");
  syncstart = 1;

  int still_running = decoders.size();
  
  if(daemonize) while(true); // loop infinitely
  else while(still_running) {
    still_running = 0;
    for( dec_iter = decoders.begin();
	 dec_iter != decoders.end();
	 ++dec_iter) {
      
      dec = *dec_iter;
      if(dec->playing) still_running++;

    }
  }

  N("quit!");

  exit(1);
}
