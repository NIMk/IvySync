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

#include <decoder.h>

Decoder::Decoder() {
  fd = 0;
  playmode = 1;
  position = 0;

  memset(buffo,0,sizeof(buffo));
}

Decoder::Decoder() {
  if(running) stop();
  if(fd) close(fd);
  quit = true;
  join();
}

