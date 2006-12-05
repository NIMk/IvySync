
// "$Id: decoder.cpp 328 2004-02-13 16:58:16Z jaromil $"

#include <sound_device.h>
#include <sound_decoder.h>
#include <utils.h>

MuseDec::MuseDec() 
  : Thread() {
    bitrate = samplerate = channels = frames = 0;
    seekable = false; err = false; eos = false;
    loaded = false;

    device = NULL;
}

MuseDec::~MuseDec() { }

bool MuseDec::play_once(SoundDevice *dev) {
  seek(0.0);
  replay = true;
  eos = false;
  device = dev;
  launch();
  return true;
}

void MuseDec::run() {
  D("Sound file decoder thread launched");
  IN_DATATYPE *buf;

  if(!device) {
    E("no device configured for decoder");
    return;
  }

  while(true) {

    while(!replay) { jsleep(0,500); }
    
    while(!eos) {
      
      buf = get_audio();
      
      if(!buf) break;
      
      device->write(buf, IN_CHUNK);
      
      //    device->flush_output();
      
    }
    replay = false;
    eos = false;
    seek(0.0);

  }

  return;
}
