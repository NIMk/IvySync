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

#include <stdio.h>
#include <unistd.h>
#include <sys/io.h>

#include <parport.h>
#include <utils.h>

#define BASEPORT 0x378 /* lp1 */

ParPort::ParPort()
  : Thread() {

  quit = false;
  button_state = false;

}

ParPort::~ParPort() {
  if( ioperm(BASEPORT, 3, 0) )
    E("error closing parallel ports");
}

bool ParPort::init() {

  if( ioperm(BASEPORT, 3, 1) ) {
    E("error accessing parallel ports");
    return false;
  }

  N("Parallel port initialized");
  return true;
  
}

void ParPort::run() {
  int status;

  A("Parallel port polling started");

  while(!quit) {
    
    jsleep(0,100); // sleep for a while

    // Read from the status port (BASE+2)
    status = inb(BASEPORT + 1);

    if( status == 120 ) {
      button_state = true;
      D("button has been pressed", status);
    }

  }
  
}

int ParPort::light(bool state) {
  if(state) {
    // Set the data signals (D0) of the port to high (255)
    outb(255, BASEPORT);
    D("switch light on");
  } else {
    // Set the data signals (D0-7) of the port to all low (0)
    outb(0, BASEPORT);    
    D("switch light off");
  }
  return state;
}

int ParPort::button_reset() {
  button_state = false;
  return 1;
}

int ParPort::button_is_pressed() {
  if(button_state) return 1;
  else return 0;
}
