#ifndef __DEV_SOUND_H__
#define __DEV_SOUND_H__

#include <portaudio.h>

#include <pipe.h>

typedef struct {
  PaDeviceID id;
  PortAudioStream *stream;
  PaDeviceInfo *info;
  Pipe *pipe;
} PaDevInfo;

typedef struct {
   PaDevInfo *input;
   PaDevInfo *output;
} PaDevices;
///< the PortAudio device descriptor , this struct just group some data used by PortAudio framework

class SoundDevice {
 public:
  SoundDevice();
  ///< the SoundDevice class constructor
  ~SoundDevice();
  ///< the SoundDevice class destructor

  /**
     Tries to open the sound device for read and/or write
     if full-duplex is requested but not supported, it returns error
     and must be called again to fallback on half-duplex mode
     
     @param read true if device is opened for reading audio
     @param write true if device is opened for writing audio
     @return true in case of success, false otherwise
  */
  bool open(bool read, bool write);
  ///< open the sound device

  bool input(bool state); ///< activate sound input
  bool output(bool state); ///< activate sound output

  void close(); ///< close the sound device
  
  int read(void *buf, int len); ///< reads audio data from the device in a buffer, len is samples
  
  int write(void *buf, int len); ///< writes audio data from a buffer to the device, len is samples
  void flush_input();
  void flush_output();

  /* TODO - should be private */
  PaDevInfo input_device; ///< portaudio input device
  PaDevInfo output_device; ///< portaudio output device
 private:
  bool pa_open(bool state,int mode);
  PaError pa_real_open(int mode);
  
  PaError err;

  PaDevices pa_dev;
  int pa_mode; ///< a switch to represent portaudio mode currently using (for noaudio,input,output or both) 
#define PaNull 0
#define PaInput 1
#define PaOutput 2


};

#endif
