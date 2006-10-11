/* MuSE - Multiple Streaming Engine
 * Copyright (C) 2004 Angelo Michele Failla aka pallotron <pallotron@freaknet.org>
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
   @file dec_ogg.h libsnd decoder
   @desc input channel: decoder implemenation
*/

#ifndef __IN_SND_H__
#define __IN_SND_H__

#include <config.h>

#ifdef HAVE_SNDFILE

#include <decoder.h>

/* libsndfile inclusion */
#include <sndfile.h>

/**
   TODO: commento da scrivere
   @class MuseDecSndFile
   @brief SndFile decoder
*/
class MuseDecSndFile:public MuseDec
{
	private:
		/* pointer to data */
		SNDFILE *sf;
		/* file information struct */
		SF_INFO sf_info_struct;		
		short snd_buffer[IN_CHUNK];
			
	public:
		/* TODO: scrivere il commento per la doc */
		MuseDecSndFile (); /* constructor */
		~MuseDecSndFile (); /* destructor */

		int load (char *file);
		bool seek (float pos);

		IN_DATATYPE *get_audio ();

};

#endif /* HAVE SNDFILE */
#endif
