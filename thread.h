/*  Pthread handling class
 *  (c) Copyright 2001 - 2006 Denis Rojo <jaromil@dyne.org>
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

#include <pthread.h>

#ifndef __THREAD_H__
#define __THREAD_H__

class Thread {

 public:
  Thread();
  virtual ~Thread();

  bool launch();
  bool running;
  bool quit;

  virtual void run() =0;

  void lock();
  void unlock();
  void join();

 private:

  pthread_t thread;
  pthread_attr_t attr;
  pthread_mutex_t mutex;
};

#endif
