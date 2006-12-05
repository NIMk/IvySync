/* MuSE - Multiple Streaming Engine
 * Copyright (C) 2004 Angelo Michele, Failla aka pallotron <pallotron@freaknet.org>
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
 *
 */

#include <string.h>

#include <sndfile_decoder.h>
#include <utils.h>

/* ----- LibSndFile input channel ----- */

MuseDecSndFile::MuseDecSndFile ()
  : MuseDec(), Entry() {

  D("MuseDecSndFile::MuseDecSndFile()");
  set_name("Snd");
  memset(&sf_info_struct, 0, sizeof(sf_info_struct));

}

MuseDecSndFile::~MuseDecSndFile (){

  D ("MuseDecSndFile::~MuseSndFile()");
  sf_close (sf);

}

int MuseDecSndFile::load (char *file) {
  
  int res; 
  /*  0 => error
   *  1 => success && seekable
   *  2 => success && !seekable
   */
  
  /* all the info about the audio file into the sf_info_struct struct */
  if(!(sf = sf_open(file, SFM_READ, &sf_info_struct))) {
    W("MuseDecSndFile:_load(): cannot open input file");
    return (0);
  }
  /*
   * this is sndfile file info structure
   * 
   * typedef struct
   *     {    sf_count_t  frames ;     // used to be called samples
   *          int         samplerate ;
   *          int         channels ;
   *          int         format ;
   *          int         sections ;
   *          int         seekable ;
   *     } SF_INFO ;	   
   */
  samplerate = sf_info_struct.samplerate;
  channels = sf_info_struct.channels;
  seekable = sf_info_struct.seekable ? true : false;
  
  D("Opened audio file: samplerate => %d, channels => %d, seekable => %s",
    samplerate, channels, seekable ? "true" : "false"); 
  
  framepos = 0;
  
  if(seekable) { 
    frametot = sf_info_struct.frames;
    D("Audio file is seekable: total frames: %d", frametot);
    res = 1; 
  }
  else res = 2;
  
  loaded = true;
  
  return (res);
  
}

IN_DATATYPE *MuseDecSndFile::get_audio () {
  
  frames = sf_read_short(sf, snd_buffer, IN_CHUNK);
  
  if(frames!=0) {
    
    framepos += frames/channels;
    fps = samplerate;
    // D("MuseDecSndFile::get_audio => Frames readed: %d/%d", framepos, frametot);
    return ((IN_DATATYPE *) snd_buffer); 	
    
  } else { framepos=0; eos = true; return (NULL); }
}


bool MuseDecSndFile::seek (float pos) {
  
  if(pos==0.0) {
    
    framepos = 0;
    sf_seek(sf, 0, SEEK_SET);
    D("MuseDecSndFile::seek => rewinded to the beginning of the track");
    
  } else	{
    
    if((framepos = sf_seek(sf, (sf_count_t)(frametot * pos), SEEK_SET))==-1) {
      D("MuseDecSndFile::seek error"); //,sf_strerror(sf));
      return false;
    }
    D("MuseDecSndFile::seek at position %d/%d", framepos, frametot);
    
  }
  
  return true;
}

