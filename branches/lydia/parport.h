/*
 * Parallel port controller
 *
 * parallel port I/O by Harry
 * C++ interface by Jaromil
 *
 *  (c) Copyright 2004-2006 Harry & Jaromil
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

#ifndef __PARPORT_H__
#define __PARPORT_H__

#include <thread.h>

class ParPort : public Thread {
  
 public:
  ParPort();
  ~ParPort();

  bool init();
  void run();
  
  int light(bool state); // switch light on/off
  
  int button_reset(); // reset button to un-pressed state
  int button_is_pressed(); // returns true if the button was pressed

 private:

  bool button_state;

};


#endif
