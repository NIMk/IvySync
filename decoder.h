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

#include <string>
#include <pthread.h>
#include <inttypes.h>

using namespace std;
using namespace __gnu_cxx;

// playmode values
#define PLAY 1
#define CONT 2
#define LOOP 3

class Decoder {

 public:
  Decoder();
  ~Decoder();

  bool init(char *dev);

  // playlist stuff
  bool prepend(char *file);
  bool insert(char *file, int pos);
  bool append(char *file);
  int playmode;
  int position;

 protected:
  bool play();
  bool stop();
  bool running;
  
 private:
  int update();

  vector<Movie*> playlist;

  string device;
  int fd;

  // posix thread stuff
  pthread_t thread;
  pthread_attr_t thread_attr;
  pthread_mutex_t mutex;

  uint8_t buffo[(1024*64)+1024]; // 64k + bound

};


#endif
