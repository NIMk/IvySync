
/*  Pvr SyncStarter
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
 * to compile this sourcecode: gcc -o pvrsync pvrsync.c -lpthreads
 * it should work on any POSIX system, including embedded hardware
 * wherever the ivtv pvr drivers can also run.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <time.h>

#define MULTITHREADED 1

#define DEBUG 1

#define MAX_DEVICES 3
#define MAX_FILES 32
#define CHUNKSIZE 1024*64

int syncstart = 0;

int players = 0;

int quitted = 0;

int act_device = 0;

struct decoder {
  // device stuff
  char dev_path[512];
  int dev_fd;

  // playlist stuff
  char *playlist[MAX_FILES];
  int pl_idx;
  FILE *pl_fd;

  // pthreads stuff
  pthread_t thread;
  pthread_attr_t thread_attr;
  pthread_mutex_t mutex;

  // The Buffer 
  char buffo[CHUNKSIZE+1024];
  
  int initialized;
  int quit;
};

struct decoder dec[MAX_DEVICES];

char *short_options = "-d:";
const struct option long_options[] = {
  { "device", required_argument, NULL, 'd'},
  {0, 0, 0, 0}
};

void jsleep(int sec, long nsec) {
  struct timespec timelap;
  timelap.tv_sec = sec;
  timelap.tv_nsec = nsec;
  nanosleep(&timelap,NULL);
}



void quitproc (int Sig) { /* signal handling */
  int c;
  fprintf(stderr,"received signal %u on process %u\n",Sig,getpid());
  for(c=0;c<players;c++) dec[c].quit = 1;
  fprintf(stderr,"quitting threads...\n");
  jsleep(1,0);
}


void flush_video(int fd) {
  // poll interface to see when is ready for the next write()

  struct pollfd fdled;
  fdled.fd = fd;
  fdled.events=POLLOUT;
  
  while( poll(&fdled,1,1000) < 1 ) { // wait infinite until ready
    if(fdled.revents & POLLOUT) return;
    else fprintf(stderr,"device %i still not ready for writing\n",fd);
  }

}

void *runner(void *arg) {
  int c;
  int num;
  int written;
  int writing;
  char *buf;
  struct decoder *pdec;

  pdec = (struct decoder*)arg;

  // try to open the device
  pdec->dev_fd =
    open(pdec->dev_path,O_WRONLY|O_NDELAY,S_IWUSR|S_IWGRP|S_IWOTH);
  if(pdec->dev_fd<0) { // error opening device
    fprintf(stderr,"error opening %s: %s\n",
	    pdec->dev_path,strerror(errno));
    //    pthread_exit(NULL);
    return NULL;
  }

  
#ifdef DEBUG

  // list the playlist
  fprintf(stderr,"decoder %s (fd:%i) playlist:\n",
	  pdec->dev_path, pdec->dev_fd);

  for(c=0;pdec->playlist[c];c++) // parse thru playlist entries
    fprintf(stderr,"%i: %s\n",c,pdec->playlist[c]);

#endif


  for(c=0;pdec->playlist[c];c++) { // play each entry

    // open playlist entry
    pdec->pl_fd = fopen( pdec->playlist[c], "rb" );
    
    // check for error here in case we loop playlists
    if(!pdec->pl_fd) {
      fprintf(stderr,"error opening %s: %s\n",
	      pdec->playlist[c],strerror(errno));
      continue;
    }

    //    fprintf(stderr,"playing %s\n", pdec->playlist[c]);

    do {   // inner loop
      
      // read a chunk of buffer in
      num = fread(pdec->buffo,1,CHUNKSIZE,pdec->pl_fd);
      
      if( feof(pdec->pl_fd) || num<1 ) { // EOF
        fprintf(stderr,"end of file %s\n",pdec->playlist[c]);
        break;
      }

      //#ifdef DEBUG
      //      else fprintf(stderr,"read %i bytes data\n",num);
      //#endif      

      written = 0;
      writing = num;
      buf = pdec->buffo;
      
      // it was locked in the main() startup of the thread
      // unlock to signal that we are ready
      if(!syncstart) {
	//	flush_video(pdec->dev_fd);
	pthread_mutex_unlock(&pdec->mutex);
      }
      // simple sync using a shared variable
      while(!syncstart) jsleep(0,1); // check every nanosecond

      // write it out all
      while(writing) { // writing loop
	

	buf += written;
        written = write(pdec->dev_fd, buf, writing);
	if(written<0) { // error on write
	  //	  fprintf(stderr,"error writing on device %s: %s\n",
	  //		  pdec->dev_path, strerror(errno));
	  continue;
	}

	//#ifdef DEBUG
	//        fprintf(stderr,"written %i bytes data\n",written);
	//#endif

	writing -= written;
	
	flush_video(pdec->dev_fd); // wait for non-blocking write

      }

    } while(num>0 && !pdec->quit); // read/write inner loop
    
    // close playlist entry
    fclose(pdec->pl_fd);
    
  } // for cycle thru playlist
  
  close(pdec->dev_fd);

  fprintf(stderr,"decoder on %s: playlist finished\n",pdec->dev_path);
  pdec->quit = 1;
  //  pthread_exit(NULL);
  return NULL;
}

int cmdline(int argc, char **argv) {
  FILE *fd;
  int c;
  int res;

  // zeroing all structs, like a constructor
  memset(dec,0,sizeof(struct decoder)*MAX_DEVICES);
  
  do { 
    res = getopt_long(argc, argv, short_options, long_options, NULL);
    
    switch(res) {
      
    case 'd':
      act_device = atoi(optarg);
      break;

    case 1:
      //fprintf(stderr,"append %s to playlist %i\n",optarg,act_device);
      fd = fopen(optarg,"rb");
      if(fd) {
	dec[act_device].playlist[ dec[act_device].pl_idx ] = strdup(optarg);
	dec[act_device].pl_idx++;
	fclose(fd);
	dec[act_device].initialized = 1;
      } else fprintf(stderr,"error: file %s not readable\n",optarg);
      
      break;

    default: break;

    }
    
  } while(res > 0);
  
}


int main(int argc, char **argv) {
  int ready;
  int c;

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
  
#ifdef MULTITHREADED
  fprintf(stderr,"running on multiple threads\n");
#else
  fprintf(stderr,"single thread player\n");
#endif



  
  for(c=0; c<MAX_DEVICES; c++) {
    
    
    if(dec[c].initialized) {    // open desired devices
      
      // formulate the device path
      sprintf(dec[c].dev_path,"/dev/video%i",16+c);

#ifdef MULTITHREADED      
      
      // initialize posix threads
      pthread_mutex_init(&dec[c].mutex,NULL);
      pthread_mutex_lock(&dec[c].mutex);

      pthread_attr_init(&dec[c].thread_attr);

      // fire up the thread
      fprintf(stderr,"firing up thread %i ...\n",c);
      pthread_create(&dec[c].thread, &dec[c].thread_attr, &runner, &dec[c]);

      
      pthread_mutex_lock(&dec[c].mutex); // blocks until thread is ready

#else
    
      runner(&dec[c]);
      break;
      
#endif
      
      players++;
      
    } // if we are here, device is correctly opened
        
  }
  
  fprintf(stderr,"%i players synchronized\n",players);

  
#ifdef MULTITHREADED

  fprintf(stderr,"Waiting 1 second before startup...");
  jsleep(1,0);
  fprintf(stderr," GO!\n");
  syncstart = 1;

  while(players != quitted) {

    // check which threads quitted and join them
    // keep the count of quits to see when all players are finished
    for(c = 0 ; c < MAX_DEVICES ; c++) { // cycle thru players
      if(dec[c].initialized)
      if(dec[c].quit) {
	fprintf(stderr,"joining thread for decoder %s\n",dec[c].dev_path);
	pthread_join(dec[c].thread,NULL);
	dec[c].quit = 0;
	quitted++;
      }
    } 

    
  }
#endif

  fprintf(stderr,"quitting!\n");

  exit(1);
}
