/*  IvySync - Video SyncStarter
 *
 *  (c) Copyright 2004 - 2005 Denis Rojo <jaromil@dyne.org>
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



#ifndef __DECODER_H__
#define __DECODER_H__

#include <iostream>
#include <string>
#include <vector>
#include <pthread.h>
#include <inttypes.h>

#include <thread.h>

using namespace std;
using namespace __gnu_cxx;

// playmode values
#define PLAY 1
#define CONT 2
#define LOOP 3
#define RAND 4

// size of video chunks read in bytes
#define CHUNKSIZE (1024*64)

class Playlist; // graphical interface

class Decoder : public Thread {

 public:
  Decoder();
  ~Decoder();

  bool init(char *dev);

  void close();

  // playlist stuff
  bool prepend(char *file); ///< prepend *file at the beginning of the playlist
  bool append(char *file); ///< append *file at the end of the playlist
  bool insert(char *file, int pos); ///< insert *file in playlist at pos
  bool remove(char *file); ///< remove the first occurrence of *file
  bool remove(int pos); ///< remove the playlist entry at pos

  // save on file
  int load();
  int save();

  int playmode; ///< PLAY, CONT, LOOP or RAND
  int position; ///< current position in playlist (read-only)
  string current; ///< path of movie currently playing

  bool play();
  bool stop();
  bool restart();
  bool playing;
  
  bool *syncstart;

  string device;  
  int device_num;

  vector<string> playlist;

  Playlist *gui; ///< pointer to the GUI, NULL if none

  bool dummy; // for dummy test run without devices

 private:
  void run();
  void update();

  void flush();


  

  int fd;
  FILE *playlist_fd;

  uint8_t buffo[(1024*64)+1024]; // 64k + 1k bound

};


#endif
