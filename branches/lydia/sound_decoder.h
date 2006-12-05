/* MuSE - Multiple Streaming Engine
 * Copyright (C) 2000-2006 Denis Rojo aka jaromil <jaromil@dyne.org>
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
 */

/**
   @file decoder.h MuSE decoder abstraction
   @desc header file to be included by decoder implementations
*/

#ifndef __SOUND_DECODER_H__
#define __SOUND_DECODER_H__

#include <inttypes.h>

#include <thread.h>
#include <linklist.h>

#define IN_DATATYPE int16_t
#define MIX_CHUNK 1152 //2048
#define IN_CHUNK MIX_CHUNK
#define IN_PIPESIZE IN_CHUNK*(sizeof(IN_DATATYPE))*64

/**
   This class should be inherited by every decoder implementation:
   it is the decoder parent class giving some common functionalities
   to its childs.
   
   Most important thing for making decoders is to implement the pure
   virtual functions of this class, inheriting all the rest.
   The pure virtual functions to be carefully implemented in a decoder are:
   - MuseDec::load
   - the destructor class for closing
   - MuseDec::seek
   - MuseDec::get_audio
   Then the decoder must also take care to set properly the following
   variables inside the load function:
   - MuseDec::samplerate
   - MuseDec::channels
   - MuseDec::bitrate
   - MuseDec::frametot
   - MuseDec::seekable
   And the following variables in the get_audio function:
   - MuseDec::frames
   - MuseDec::fps
   - MuseDec::eos
   - MuseDec::err

   For example decoder implementations, please refer to:
   - MuseDecMp3 class implemented in dec_mp3.h and dec_mp3.cpp
   - MuseDecOgg class implemented in dec_ogg.h and dec_ogg.cpp
   - MuseDecSnd class implemented in dec_snd.h and dec_snd.cpp

   @brief decoder parent abstraction class
*/

class SoundDevice;

class MuseDec: public Thread {

  public:

  /**
     The decoder implementations inheriting from this class can
     use their constructor to initialize their variables and to
     fill up the MuseDec::name buffer with their identification.
    
     @brief decoder parent class constructor */
  MuseDec();

  /**
     
     A decoder implementation should take care to close all files and
     free all buffers in the destructor.

     @brief decoder parent class destructor */
  virtual ~MuseDec();

  /**
     Open up a filename (full path) and makes it ready for decoding,
     the filename or url can be formed in different ways, depending
     on the decoder implementation.

     This is a pure virtual function: needs to be implemented in decoders.

     @brief open file in decoder
     @param file full pathname for file, or url accepted by the decoder
     @return 0 on error, otherwise
             - 1 = success, channel is seekable
	     - 2 = success, channel is not seekable
  */
  virtual int load(char *file) = 0; /* open filename */

  bool loaded; ///< should be set to true by the implemention on succesful load

  /**
     Seek position over the audio data available to an opened channel.

     This operation is only possible if the channel is seekable (see the
     flag in this class and the return code of MuseDec::load).

     This is a pure virtual function: needs to be implemented in decoders.

     @brief seek to a position
     @param pos floating point value from 0.0 to 1.0
     @return true on success, false otherwise */
  virtual bool seek(float pos) = 0; /* seek to position from 0.0 1.0 */

  /**
     Decode another chunk of audio for the channel at the current position,
     this function is implementing the low-level decoder functionalities
     to obtain the audio pcm to be mixed.

     The audio will be then resampled at a common rate and mixed by MuSE.

     This is a pure virtual function: needs to be implemented in decoders.
     
     @brief decode a chunk of channel audio
     @return pointer to decoded pcm buffer */
  virtual IN_DATATYPE *get_audio() = 0;  /* decode audio */



  // =====================================================================
  // DECODER PARENT FUNCTIONS
  
  void run(); ///< thread runner

  /**
     Playback once the loaded file on a device, spawning a thread

     @brief playback once the audio file
     @return true on success, false otherwise */
  bool play_once(SoundDevice *dev);
  bool replay;

  SoundDevice *device;

  /**
   * the following variables describe the audio returned by
   * MuseDec::get_audio and must be setted up by the decoder implementation.
   */
  int samplerate; ///< samplerate of audio decoded
  int channels; ///< number of audio channels decoded
  int bitrate; ///< bitrate of the compressed audio being decoded
  int frames; ///< quantity of audio frames (16bit words) decoded
  int framepos; ///< position offset on the frames
  int frametot; ///< total frames in opened audio (if seekable, othwrwise 0)
  int fps; ///< samplerate / frames quantity
  bool seekable; ///< true if the channel audio is seekable
  bool eos; ///< true on end of stream reached
  bool err; ///< true when an error occurred during audio decoding
  ///////////////////////////////////////////////////////////




};    

#endif
