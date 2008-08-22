/*  Generic utils
 *  (c) Copyright 2001-2006 Denis Roio aka jaromil <jaromil@dyne.org>
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
#include <string.h>
#include <cstdio>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

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

void N(const char *format, ...) {
  va_list arg;
  va_start(arg, format);

  vsnprintf(msg, 254, format, arg);
  fprintf(stderr,"[*] %s\n",msg);
   
  va_end(arg);
}

void D(const char *format, ...) {
  if(verbosity>=FUNC) {
    va_list arg;
    va_start(arg, format);
    
    vsnprintf(msg, 254, format, arg);
    fprintf(stderr,"[F] %s\n",msg);
    
    va_end(arg);
  }
}

void E(const char *format, ...) {
  va_list arg;
  va_start(arg, format);
  
  vsnprintf(msg, 254, format, arg);
  fprintf(stderr,"[!] %s\n",msg);

  va_end(arg);
}

void A(const char *format, ...) {
  va_list arg;
  va_start(arg, format);
  
  vsnprintf(msg, 254, format, arg);
  fprintf(stderr," .  %s\n",msg);
  
  va_end(arg);
}

void W(const char *format, ...) {
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

// jaromil's chomp
#define MAX_CHOMP_SIZE 1024
void chomp(char *str) {
  size_t len; //, ilen;
  char tmp[MAX_CHOMP_SIZE], *p = str;
  
  memset(tmp,'\0',MAX_CHOMP_SIZE);
  
  /* eliminate space and tabs at the beginning */
  while (*p == ' ' || *p == '\t') p++;
  strncpy(tmp, p, MAX_CHOMP_SIZE);
  
  /* point *p at the end of string */
  len = strlen(tmp); 
  p = &tmp[len-1];
  
  while ((*p == ' ' || *p == '\t' || *p == '\n') && len) {
    *p = '\0'; p--; len--;
  }

  strncpy(str, tmp, MAX_CHOMP_SIZE);
}

void get_time(char *f, struct tm *tt) {
  strptime(f,"%d%b%y-%H%M",tt);
  return;
}

char *mark_time() {
  static char dm[32];
  struct tm *tm;
  time_t now;

  now = time(NULL);
  tm = localtime(&now);
  strftime(dm,31,"%d%b%y-%H%M",tm);
  return(dm);
}
  
#ifdef linux
#include <sched.h>
/* sets the process to "policy" policy,  if max=1 then set at max priority,
   else use min priority */

bool set_rtpriority(bool max) {
  struct sched_param schp;
  // set the process to realtime privs

  memset(&schp, 0, sizeof(schp));
  
  if(max) 
    schp.sched_priority = sched_get_priority_max(SCHED_RR);
  else
    schp.sched_priority = sched_get_priority_min(SCHED_RR);
  
  if (sched_setscheduler(0, SCHED_RR, &schp) != 0)
    return false;
  else
    return true;
}
#endif
