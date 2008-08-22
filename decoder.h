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



#ifndef __DECODER_H__
#define __DECODER_H__

// for large file support (64bit file offsets)
#define _LARGEFILE_SOURCE 1

/* On many architectures both off_t and long are 32-bit types, but
   compilation with #define _FILE_OFFSET_BITS 64 will turn off_t into
   a 64-bit type. - see man fseeko(3) */
#define _FILE_OFFSET_BITS 64


#include <iostream>
#include <pthread.h>
#include <inttypes.h>

#include <linklist.h>
#include <thread.h>



// playmode values
#define PLAY   1
#define CONT   2
#define LOOP   3
#define RAND   4
#define SINGLE 5

// maximum path lenght
#define MAXPATH 512

#ifdef WITH_GUI
class Playlist; // graphical interface
#endif


class Decoder : public Thread, public Entry {

 public:
  Decoder();
  ~Decoder();

  bool init(const char *dev);

  bool setup(bool *sync, int bufsize);

  void close();

  // playlist stuff
  bool prepend(char *file); ///< prepend *file at the beginning of the playlist
  bool append(char *file); ///< append *file at the end of the playlist
  bool insert(char *file, int pos); ///< insert *file in playlist at pos
  //  bool remove(char *file); ///< remove the first occurrence of *file
  bool remove(int pos); ///< remove the playlist entry at pos
  bool empty(); ///< empty all the playlist discarding all entries

  // save on file
  int load();
  int save();

  int playmode; ///< PLAY, CONT, LOOP or RAND
  int position; ///< current position in playlist (read-only)


  bool play();
  bool stop();
  bool pause();
  bool clear();

  int   getpos();
  void  setpos(int pos);

  off64_t getoffset();
  void setoffset(off64_t pos);

  /** state flags for use in the inner loop
      the following booleans are changed by asynchronous calls
      then behaviour is synched and executed in the main loop */
  bool playing;
  bool stopped;
  
  bool *syncstart;

  char device[MAXPATH];
  int device_num;

  Linklist playlist;
  Entry *current; ///< path of movie currently playing

#ifdef WITH_GUI
  Playlist *gui; ///< pointer to the GUI, NULL if none
#endif

  bool dummy; // for dummy test run without devices

 private:
  void run();
  void update();

  void flush();

  off64_t filesize; // current file playing, size in bytes
  off64_t filepos; // current file playing, position in bytes
  off64_t newfilepos; // new position to skip in file

  int fd;
  FILE *playlist_fd;

//  uint8_t buffo[CHUNKSIZE+1024]; // + 1k bound
  uint8_t *buffo;
  int buffo_size;

};


#endif
