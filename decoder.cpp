/*  IvySync - Video SyncStarter
 *
 *  (c) Copyright 2004 - 2006 Denis Rojo <jaromil@dyne.org>
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
 */

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/select.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <decoder.h>
#include <utils.h>
#include <gui.h>

Decoder::Decoder()
  : Thread() {
  fd = 0;
  playmode = PLAY;
  position = -1;
  playing = false;
  dummy = false;
  gui = false;
  quit = true;

  filesize = 0L;
  filepos = 0L;
  newfilepos = 0L;

  buffo = NULL;
}

Decoder::~Decoder() {
  close();
  free(buffo);
  quit = true;
}

bool Decoder::init(char *dev) {
  int len;

  if(dummy) {

    N("running in dummy test run - no device opened");
    device = dev;

  } else {

    fd = ::open(dev, O_WRONLY|O_NDELAY,S_IWUSR|S_IWGRP|S_IWOTH);
    if(fd<0) {
      D("error opening device %s: %s",dev,strerror(errno));
      return false;
    }

  }

  // parse the last 2 cyphers of the device path
  device = dev;
  len = strlen(dev);
  // last two chars of the device name are the number
  device_num = atoi(&dev[len-2]);


  return true;
}

bool Decoder::setup(bool *sync, int bufsize) {

  // save the syncstarter flag
  syncstart = sync;
  
  if(buffo) free(buffo);
  
  buffo = (uint8_t*) calloc( bufsize+1, 1024); // +1 safety bound
  if(!buffo) {
    E("fatal error: can't allocate %uKB of memory for decoder", bufsize);
    return(false);
  }
  
  buffo_size = bufsize*1024;

  quit = false;

  return(true);
			     
}

void Decoder::close() {
  if(playing) stop();
  quit = true;
  if(running) {
    D("thread was running, waiting to join...");
    join();
  }
  if(fd) ::close(fd);
}

void Decoder::update() {
  if(position<0) { // first time we play from the playlist
    position = 0;
  } else {
    
    switch(playmode) {

    case PLAY: // next

      if( position+1 > playlist.size()-1 ) stop();
      else position++;

      break;

    case CONT: // next or first if at the end

      if( position+1 > playlist.size()-1 ) position = 1;
      else position++;

      break;

    case LOOP:
      // play the same again
      break;

    case RAND:
      // play a random one
      break;

    default:
      stop();
      // just stop
      break;
    }

  }
  
  // current movie is now in playlist[position];

  // refresh the GUI if present
  if(gui) gui->refresh();

}



void Decoder::run() {
  int in, written, writing;
  uint8_t *buf;
  string movie;

  if(!fd && !dummy) {
    E("thread %u falling down: no device opened",pthread_self());
    return;
  }
  
  running = true;

  D("thread %u launched",pthread_self());

  // set max realtime priority
//  if( set_rtpriority(true) )
//    A("thread %u running on max realtime priority",pthread_self());
  
  while(!quit) {

    update();

    // if is not playling, sleep
    while(!playing && !quit)
      jsleep(0,1);
    if(quit) break;
    ///////////////////////////

    movie = playlist[position];

    playlist_fd = fopen( movie.c_str(), "rb" );
    if(!playlist_fd) {
      E("can't open %s: %s (%i)", movie.c_str(), strerror(errno), errno);

      if(errno==27) { // EOVERFLOW - file too large on Linux/Glibc

	int tmpfd;
	tmpfd = open( movie.c_str(), O_RDONLY|O_LARGEFILE);

	if(!tmpfd) {
	  E("failed opening with largefile support: %s",strerror(errno));
	  update();
	  continue;
        } else playlist_fd = fdopen( tmpfd , "rb" );

      } else {
        update();
        continue;
      }
    }

    N("now playing %s",movie.c_str());
    
    // read the total length of movie file
    fseek(playlist_fd, 0L, SEEK_END);
    filesize = ftell(playlist_fd);
    fseek(playlist_fd, 0L, SEEK_SET);
    filepos = 0L; // set position at the beginning
    
    A("movie length: %lu (bytes)",filesize);

    do { // inner reading loop

      // update the GUI
      if(gui) gui->refresh();
      
      // if is not playling, sleep
      while(!playing && !quit) {
	jsleep(0,200);
      }
      

      if(quit) break;
      ///////////////////////////

      in = fread(buffo, 1, buffo_size, playlist_fd);
      if( feof(playlist_fd) || in<1 ) { // EOF
	D("end of file: %s",movie.c_str());
	break;
      }
      
      written = 0;
      writing = in;
      buf = buffo;
      
      if(!*syncstart) {
	unlock();
	while(!*syncstart) jsleep(0,1); // check every nanosecond
      }
      
      while(writing) { // writing loop
	
	buf += written;

	if(quit) break;

	if(dummy) // emulation mode with no device (for devel)
	  written = writing; 
	else
	  written = ::write(fd, buf, writing);

	if(written<0) // error on write
	  continue;
	else
	  filepos += written;

	writing -= written;
	
	flush();
	
      }
      
    } while(in>0 && !quit); // read/write inner loop
    
    fclose(playlist_fd);
    playlist_fd = 0;

  } // run() thread loop

  if(playlist_fd)
    fclose(playlist_fd); // be sure we close
  playlist_fd = 0;

  D("thread %u finished", pthread_self());
  return;
}

void Decoder::flush() {
  struct pollfd fdled;
  fdled.fd = fd;
  fdled.events=POLLOUT;
  
  while( poll(&fdled,1,1000) < 1 ) { // wait infinite until ready
    if(fdled.revents & POLLOUT) return;
    else {
      W("device %i still not ready for writing",fd);
      if(quit) return;
    }
  }

  // if there is a seek to do, do it now
  if(newfilepos > 0L) {
    D("seeking to new position %lu", newfilepos);
    fseek(playlist_fd, newfilepos, SEEK_SET);
    filepos = newfilepos;
    newfilepos = 0;
  }
}

bool Decoder::play() {
  playing = true;
  return playing;
}
bool Decoder::stop() {
  playing = false;
  return playing;
}
bool Decoder::restart() {
  playing = false;
  position = 0;
  return true;
}

int Decoder::getpos() {
  // filesize : 100 = filepos : x
  // filesize : filepos = 100 : x
  int percent;

  percent = (filepos * 100) / filesize;
  //  D("movie %s at position %u %% (%lu byte)",
  //    movie.c_str(), percent, filepos);
  return percent;
}

void Decoder::setpos(int pos) {
  // filesize : 100 = x : pos

  newfilepos = (filesize * pos) / 100;

  D("Decoder::setpos(%u) : newfilepos = %lu",
    pos, newfilepos);
}
    

bool Decoder::prepend(char *file) {
  playlist.insert( playlist.begin(), file );
  return true;
}

bool Decoder::append(char *file) {
  playlist.push_back(file);
  return true;
}

bool Decoder::insert(char *file, int pos) {
  if(pos > playlist.size() ) {

    // playlist is smaller than pos: append at the end
    playlist.push_back(file);
    
  } else {

    vector<string>::iterator pl_iter;
    int c;
    for( pl_iter = playlist.begin(), c=1;
	 c<pos; ++pl_iter, c++ );
    playlist.insert( pl_iter, file );

  }
  return true;
}

/*
bool Decoder::remove(char *file) {
  A("TODO: Decoder::remove(char *file)");
  return true;
}
*/
bool Decoder::remove(int pos) {
  vector<string>::iterator pl_iter;
  int c;

  for( pl_iter = playlist.begin(), c=1;
       pl_iter != playlist.end();
       ++pl_iter, c++ )
    if(c==pos) {
      playlist.erase(pl_iter);
      return true;
    }

  return false;
}

static time_t now_epoch;
static struct tm     now;

int playlist_selector(const struct dirent *dir) {
  char today_str[32];

  strftime(today_str,31,"%d%b%y",&now);

  if( strstr(dir->d_name, today_str) )
    return 1;
  
  if( strstr(dir->d_name, "video") )
    return 1;

  return 0;
}
  
  
  
int Decoder::load() {
  // load the playlist from the .ivysync/ directory
  // renders a date string of today in the format of DDMMMYY-HHMM (12Aug)
  // if the playlist .ivysync/DDMMM*-videoNN is there load that one
  // otherwise fallback on the .ivysync/videoNN playlist
  // if that is not even there then we don't have a playlist.
  FILE *fd;
  char path[512];
  char line[1024];
  int c = 0;
  struct stat st;

  struct tm pltime;
  struct tm plseltime;

  struct dirent **filelist;
  int found;
  
  char ThePlaylist[512];
  char videodev[64];
  char *home = getenv("HOME");

  snprintf(path,511,"%s/.ivysync",home);
  if( stat(path, &st) != 0) {
    D("no saved playlists in %s: %s",path,strerror(errno));
    return -1;
  }
    
  // when we are now
  now_epoch = time(NULL);
  localtime_r( &now_epoch, &now );
  snprintf(videodev,63,"video%u",device_num);
  // use the default playlist
  snprintf(ThePlaylist,511,"video%u",device_num);

  // scan the directory for scheduled playlists starting with date
  found = scandir(path, &filelist, playlist_selector, alphasort);
  if(found < 0) {
    E("playlist scandir: %s",strerror(errno));
    return -1;
  }
 
  // setup time selection of the latest playlist of today
  memcpy(&plseltime, &now, sizeof(struct tm));
  plseltime.tm_hour = 0;
  plseltime.tm_min = 0;

  // in filelist[] we have all playlists of today
  // now need to sort out the ones that are not from this device
  // and the ones that are in the future (hour and minutes)
  while(found--) {

    D("checking playlist for today %s", filelist[found]->d_name);
    // eliminate the ones that are not for this device
    if( ! strstr( filelist[found]->d_name, videodev ) ) continue;

    // read and check the exact time on the filename
    // in case the playlist filename doesn't starts with video*
    if ( filelist[found]->d_name[0] == 'v') {
      
      snprintf(ThePlaylist,511,"%s",filelist[found]->d_name);

    } else {

      get_time( filelist[found]->d_name, &pltime );
    
      // skip if we already have a more recent playlist
      if(plseltime.tm_hour > pltime.tm_hour) continue;
      else if(plseltime.tm_hour == pltime.tm_hour)
        if(plseltime.tm_min >= pltime.tm_min) continue;
    
      if(now.tm_hour > pltime.tm_hour) {
      
        D("this playlist is actual, we're going to use this");
        snprintf(ThePlaylist,511,"%s",filelist[found]->d_name);
        memcpy(&plseltime,&pltime,sizeof(struct tm));

      } else if(now.tm_hour == pltime.tm_hour) {
      
	// same hour, let's check the minutes
	if(now.tm_min >= pltime.tm_min) {
	  
	  D("this playlist is scheduled right now");
	  snprintf(ThePlaylist,511,"%s",filelist[found]->d_name);
	  memcpy(&plseltime,&pltime,sizeof(struct tm));
	  
	}
      } else D("this playlist will be activated later");
    } 
  }

  snprintf(path,511,"%s/.ivysync/%s",home,ThePlaylist);

  fd = fopen(path,"r");
  if(!fd) {
    E("can't load playlist %s: %s", path, strerror(errno));
    return -1;
  }

  A("reading from playlist file %s",path);
  while( fgets(line,1023,fd) ) {
    if( feof(fd) ) break;

    chomp(line);
    if( append(line) ) {
      c++;
      D("%u+ %s",c,line);
    }
  }
  fclose(fd);
  return c;
}

int Decoder::save() {
  FILE *fd;
  char *home = getenv("HOME");
  char path[512];
  int c;
  struct stat st;

  vector<string>::iterator pl_iter;
  string pl;

  // create the configuration directory if doesn't exist
  snprintf(path,511,"%s/.ivysync",home);
  if( stat(path, &st) != 0) {
    if(errno==ENOENT) mkdir(path,0744);
    else {
      E("error saving in %s: %s",path, strerror(errno));
      return -1;
    }
  }

  snprintf(path,511,"%s/.ivysync/video%u",home,device_num);
  fd = fopen(path,"w+");
  if(!fd) {
    E("can't save to %s: %s", path, strerror(errno));
    return -1;
  }
  D("saving to configuration file %s",path);
  for(c=1, pl_iter = playlist.begin();
      pl_iter != playlist.end();
      ++pl_iter, c++) {

    pl = *pl_iter;
    fputs(pl.c_str(),fd);
    fputs("\n",fd);
    D("%u. %s",c,pl.c_str());
  }
  fflush(fd);
  fclose(fd);
  return c;
}
