
// "$Id: decoder.cpp 328 2004-02-13 16:58:16Z jaromil $"

#include <sound_decoder.h>
#include <utils.h>

MuseDec::MuseDec() 
  : Thread() {
    bitrate = samplerate = channels = frames = 0;
    seekable = false; err = false; eos = false;
    loaded = false;
}

MuseDec::~MuseDec() { }

void MuseDec::run() {
  W("TODO: multi-threaded play of channels");
  return;
}
