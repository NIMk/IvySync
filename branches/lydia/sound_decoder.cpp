
// "$Id: decoder.cpp 328 2004-02-13 16:58:16Z jaromil $"

#include <sound_decoder.h>
#include <utils.h>

MuseDec::MuseDec() {
    bitrate = samplerate = channels = frames = 0;
    seekable = false; err = false; eos = false;
    loaded = false;
    if(pthread_mutex_init (&mutex,NULL) == -1)
      E("%i:%s error initializing POSIX thread mutex",
	    __LINE__,__FILE__);
}

MuseDec::~MuseDec() {
  if(pthread_mutex_destroy(&mutex) == -1)
    E("error destroying POSIX thread mutex",
	  __LINE__,__FILE__);
}
