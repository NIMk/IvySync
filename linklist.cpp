/*  C++ Linked list class, threadsafe (boolean is atom)
 *
 *  (c) Copyright 2001-2004 Denis Roio aka jaromil <jaromil@dyne.org>
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
 * "$Id: linklist.cpp 274 2005-05-04 13:29:10Z kysucix $"
 *
 -------------------------------------------------------------------------
   linked list container class

   NOTE: add and rem don't take care of deleting pointers
   that has to be done by the process that creates them and
   knows which inheriting class they are (delete is done in main)
*/

#include <stdlib.h>
#include <string.h>

#include <utils.h>
#include <linklist.h>


Linklist::Linklist() {
  length = 0;
  first = NULL;
  last = NULL;
#ifdef THREADSAFE
  pthread_mutex_init(&mutex,NULL);
#endif
  
//  unlock();
}

Linklist::~Linklist() {
  clear();
}

/* adds one element at the end of the list */
void Linklist::append(Entry *addr) {
  Entry *ptr = NULL;
  if(!addr) {
    W("Linklist::append called with NULL argument");
    return;
  }
  if(addr->list) addr->rem();
#ifdef THREADSAFE
  lock();
#endif

  if(!last) { /* that's the first entry */
    last = addr;
    last->next = NULL;
    last->prev = NULL;
    first = last;
  } else { /* add the entry to the end */
    ptr = last;
    ptr->next = addr;
    addr->next = NULL;
    addr->prev = ptr;
    last = addr;
  }
  /* save the pointer to this list */
  addr->list = this;
  length++;
#ifdef THREADSAFE
  unlock();
#endif
}

void Linklist::prepend(Entry *addr) {
  Entry *ptr = NULL;
  if(addr->list) addr->rem();
#ifdef THREADSAFE
  lock();
#endif
  
  if(!first) { /* that's the first entry */
    first = addr;
    first->next = NULL;
    first->prev = NULL;
    last = first;
  } else { /* add an entry to the beginning */
    ptr = first;
    ptr->prev = addr;
    addr->next = ptr;
    addr->prev = NULL;
    first = addr;
  }
  addr->list = this;
  length++;
#ifdef THREADSAFE
  unlock();
#endif
}


// inserts an element after the given one
void Linklist::insert_after(Entry *addr, Entry *pos) {
  
  // take it out from other lists
  if(addr->list) addr->rem();

#ifdef THREADSAFE
  lock();
#endif
  if(pos->next) {
    pos->next->prev = addr;
    addr->next = pos->next;
  } else last = addr; // it's the last
  
  addr->prev = pos;
  pos->next = addr;

  length++;  
  addr->list = this;

#ifdef THREADSAFE
  unlock();
#endif
}


/* adds an element at the position specified
   if pos is out of bounds adds it at the beginning or the end
   the element occupying allready the position slides down 
   THIS FUNCTION IS NOT YET RELIABLE
*/
void Linklist::insert(Entry *addr, int pos) {
  if(length<=pos) { /* adds it at the end */
    append(addr);
    return;
  } else if(pos<=1) {
    prepend(addr);
    return;
  }

  if(addr->list) addr->rem();

  Entry *ptr = pick(pos);

#ifdef THREADSAFE
  lock();
#endif
  ptr->prev->next = addr;
  addr->prev = ptr->prev;
  
  ptr->prev = addr;
  addr->next = ptr;
  
  length++;
  addr->list = this;
#ifdef THREADSAFE
  unlock();
#endif
}

/* clears the list
   i don't delete filters here because they have to be deleted
   from the procedure creating them. so this call simply discards
   the pointers stored into the linked list. OBJECTS ARE NOT FREED */
void Linklist::clear() {
#ifdef THREADSAFE
  lock();
#endif
  sel(0);
  length = 0;
  first = NULL;
  last = NULL;
#ifdef THREADSAFE
  unlock();
#endif
}

/* takes one element from the list
   === STARTING FROM 1 ===
   returns NULL if called with pos=0 or pos>length
   returns Entry pointer otherwise 
   this function is then overloading the operator[]
*/
Entry *Linklist::pick(int pos) {
  if((length<pos)||(pos<1)) return(NULL);
  if(pos==1) return(first);
  if(pos==length) return(last);

  Entry *ptr = first;
  register int c;
  for(c=1;c<pos;c++) {
    if(!ptr) break;
    ptr = ptr->next;
  }

  return(ptr);
}

/* search the linklist for the entry matching *name
   returns the Entry* on success, NULL on failure */
Entry *Linklist::search(char *name) {
  Entry *ptr = first;
  while(ptr) {
    if( strcasecmp(ptr->name,name)==0 ) break;
    ptr = ptr->next;
  }
  return(ptr);
}    
/* searches all the linklist for entries starting with *needle
   returns a list of indexes where to reach the matches */
int *Linklist::completion(char *needle) { 
  register int c;
  int found;
  int len = strlen(needle);

  /* cleanup */
  memset(compbuf,0,MAX_COMPLETION);

  /* check it */
  Entry *ptr = first;
  if(!ptr) return compbuf;

  for( found=0, c=1 ; ptr ; c++ , ptr=ptr->next ) {
    if(!len) { // 0 lenght needle: return the full list
      compbuf[found] = c;
      found++;
    } else if( strncasecmp(needle,ptr->name,len)==0 ) {
      compbuf[found] = c;
      found++;
    }
  }

  D("completion found %i hits",found);
  return compbuf;
}


/* this function is a wrapper around Entry::up()
   better to use that if you have a pointer to your Entry */
bool Linklist::moveup(int pos) {
  Entry *p = pick(pos);
  if(!p) return(false);
  return( p->up() );
}
bool Linklist::movedown(int pos) {
  Entry *p = pick(pos);
  if(!p) return(false);
  return( p->down() );
}
bool Linklist::moveto(int num, int pos) {
  Entry 
    *p = pick(num);
  if(!p) return(false);
  return( p->move(pos) );
}
/* removes one element from the list */
void Linklist::rem(int pos) {
  Entry *ptr = pick(pos);
  if(ptr==NULL) return;
  ptr->rem();
}
  
/* selects ONLY ONE, deselects the others
   use Entry::sel() if you want to do multiple selects */
void Linklist::sel(int pos) {
  int c;
  Entry *ptr = last;

  if(pos>length) return;

  for(c=length;c>0;c--) {
    if(c==pos) ptr->sel(true);
    else ptr->sel(false);
    ptr = ptr->prev;
  }
}

/* returns the last one selected
   this is supposed to be used with single selections */
Entry *Linklist::selected() {  
  int c;
  Entry *ptr = last;
  for(c=length;c>0;c--) {
    if(ptr->select) return ptr;
    ptr = ptr->prev;
  }
  return NULL;
}

Entry::Entry() {
  next = NULL;
  prev = NULL;
  list = NULL;
  data = NULL;
  select = false;
  strcpy(name,"noname");
}

Entry::~Entry() {
  rem();
  if(data) free(data);
}

void Entry::set_name(char *nn) {
  strncpy(name,nn,MAX_ENTRY_NAME-1);
}

bool Entry::up() {
  if(!prev || !list) return(false);
#ifdef THREADSAFE
  list->lock();
#endif

  Entry *tprev = prev,
    *tnext = next,
    *pp = prev->prev;

  if(!next)
    list->last = prev;

  if(tnext)
    tnext->prev = tprev;

  next = tprev;
  prev = pp;
  tprev->next = tnext;
  tprev->prev = this;

  if(pp)
    pp->next = this;

  if(!prev)
    list->first = this;

#ifdef THREADSAFE
  list->unlock();
#endif
  return(true);
}

bool Entry::down() {
  if(!next || !list) return(false);
#ifdef THREADSAFE
  list->lock();
#endif

  Entry *tprev = prev,
    *tnext = next,
    *nn = next->next;

  if(!prev)
    list->first = next;

  if(tprev)
    tprev->next = tnext;

  prev = tnext;
  next = nn;
  tnext->prev = tprev;
  tnext->next = this;
  if(nn)
    nn->prev = this;

  if(!next)
    list->last = this;

#ifdef THREADSAFE
  list->unlock();
#endif
  return(true);
}

bool Entry::move(int pos) {
  D("Entry::move(%i) - NEW LINKLIST MOVE, TRYING IT...");
  if(!list) return(false);
#ifdef THREADSAFE
  list->lock();
#endif

  Entry *tn, *tp;

  Entry *swapping = list->pick(pos);
  if(swapping == this) return(true);
  if(!swapping) return(false);

  tn = swapping->next;
  tp = swapping->prev;

  swapping->next = next;
  swapping->prev = prev;
  if(next) next->prev = swapping;
  else list->last = swapping;
  if(prev) prev->next = swapping;
  else list->first = swapping;

  next = tn;
  prev = tp;
  if(next) next->prev = this;
  else list->last = this;
  if(prev) prev->next = this;
  else list->first = this;

#ifdef THREADSAFE
  list->unlock();
#endif
  D("LINKLIST MOVE RETURNS SUCCESS");

  return(true);
}

void Entry::rem() {
  bool lastone = false;
  if(!list) return;
#ifdef THREADSAFE
  list->lock();
#endif

  if(next) { // if there is a next
    next->prev = prev; // link it to the previous
    next->select = select; // inherit selection
  } else {
    list->last = prev; // else just make it the last
    lastone = true;
  }

  if(prev) { // if there is a previous
    prev->next = next; // link it to the next
    if(lastone) prev->select = select;
  } else list->first = next; // else just make it a first
  
  list->length--;
#ifdef THREADSAFE
  list->unlock();
#endif

  list = NULL;
}

void Entry::sel(bool on) {
  select = on;
}
