/*  Generic utils, used in Fakiir
 *  (c) Copyright 2001-2005 Denis Roio aka jaromil <jaromil@dyne.org>
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

#include <iostream>
#include <string>
#include <cstdio>
#include <stdlib.h>
#include <stdarg.h>

#include <utils.h>

#define MAX_DEBUG 3

#define FUNC 2 // se il debug level e' questo ci sono le funzioni chiamate
#define WARN 1 // se il debug level e' >= a questo ci sono i warning

static char msg[255];

static int verbosity = 0;

void set_debug(int lev) {
  lev = lev<0 ? 0 : lev;
  lev = lev>MAX_DEBUG ? MAX_DEBUG : lev;
  verbosity = lev;
}

int get_debug() {
  return(verbosity);
}

void N(char *format, ...) {
  va_list arg;
  va_start(arg, format);

  vsnprintf(msg, 254, format, arg);
  fprintf(stderr,"[*] %s\n",msg);
   
  va_end(arg);
}

void D(char *format, ...) {
  if(verbosity>=FUNC) {
    va_list arg;
    va_start(arg, format);
    
    vsnprintf(msg, 254, format, arg);
    fprintf(stderr,"[F] %s\n",msg);
    
    va_end(arg);
  }
}

void E(char *format, ...) {
  va_list arg;
  va_start(arg, format);
  
  vsnprintf(msg, 254, format, arg);
  fprintf(stderr,"[!] %s\n",msg);

  va_end(arg);
}

void A(char *format, ...) {
  va_list arg;
  va_start(arg, format);
  
  vsnprintf(msg, 254, format, arg);
  fprintf(stderr," .  %s\n",msg);
  
  va_end(arg);
}

void W(char *format, ...) {
  if(verbosity>=WARN) {
    va_list arg;
    va_start(arg, format);
    
    vsnprintf(msg, 254, format, arg);
    fprintf(stderr,"[W] %s\n",msg);
  
    va_end(arg);
  }
}

void jsleep(int sec, long nsec) {
  struct timespec timelap;
  timelap.tv_sec = sec;
  timelap.tv_nsec = nsec;
  nanosleep(&timelap,NULL);
}
