/* MuSE - Multiple Streaming Engine
 * SoundDevice class interfacing Portaudio PABLIO library
 * Copyright (C) 2004-2005 Denis Roio aka jaromil <jaromil@dyne.org>
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
 
 "$Id: dev_sound.cpp 760 2006-03-16 23:37:40Z xant $"
 
 */

#include <stdlib.h>
#include <config.h>

#include <dev_sound.h>
#include <jutils.h>
#include <generic.h>

#define PA_SAMPLE_TYPE paFloat32
#define PA_SAMPLES_PER_FRAME 2
#define PA_NUM_SECONDS 5
#define FRAMES_PER_BUFFER   (64)
#define PA_PIPE_SIZE MIX_CHUNK*sizeof(PA_SAMPLE_TYPE)*64 

#define INPUT_DEVICE  Pa_GetDefaultInputDeviceID()
#define OUTPUT_DEVICE Pa_GetDefaultOutputDeviceID()


#ifdef HAVE_JACK
int dev_jack_process(jack_nframes_t nframes, void *arg) {
  
  jack_nframes_t opframes;
  SoundDevice *dev = (SoundDevice*)arg;
  if(!dev->jack) return 0; // just return
  
  // take output from pipe and send it to jack
  dev->jack_out_buf = (jack_default_audio_sample_t*)
    jack_port_get_buffer(dev->jack_out_port,nframes);
  opframes = dev->jack_out_pipe->read
    (nframes * sizeof(float) * 2 , dev->jack_out_buf);
  
  // take input from jack and send it in pipe
  dev->jack_in_buf = (jack_default_audio_sample_t*)
    jack_port_get_buffer(dev->jack_in_port,nframes);
  dev->jack_in_pipe->write // does the float2int conversion in one pass
    (nframes * sizeof(float) * 2 , dev->jack_in_buf);
  
  return 0;
}

void dev_jack_shutdown(void *arg) {
  SoundDevice *dev = (SoundDevice*)arg;
  // close the jack channels
  dev->jack = false;
  jack_port_unregister(dev->jack_client, dev->jack_in_port);
  jack_port_unregister(dev->jack_client, dev->jack_out_port);
  jack_deactivate(dev->jack_client);
  delete dev->jack_in_pipe;
  delete dev->jack_out_pipe;
}
#endif


SoundDevice::SoundDevice() {
  memset(&input_device,0,sizeof(input_device));
  memset(&output_device,0,sizeof(output_device));
  pa_dev.input = &input_device;
  pa_dev.output = &output_device;
  input_device.pipe = new Pipe(PA_PIPE_SIZE);
  input_device.pipe->set_block(false,false);
  output_device.pipe = new Pipe(PA_PIPE_SIZE);
  output_device.pipe->set_block(true,false);
  input_device.pipe->set_output_type("copy_float_to_int16");
  output_device.pipe->set_input_type("copy_int16_to_float");
  jack = false;
  jack_in = false;
  jack_out = false;
}

SoundDevice::~SoundDevice() {
close();
}

static int pa_process( void *inputBuffer, void *outputBuffer, 
	unsigned long framesPerBuffer, 
	PaTimestamp outTime, void *userData )
{
int i,n;
void *rBuf;
int readBytes;
PaDevices *dev = (PaDevices *)userData;
long len = framesPerBuffer * (PA_SAMPLES_PER_FRAME*sizeof(PA_SAMPLE_TYPE));
  if(inputBuffer != NULL) { /* handle input from soundcard */
	if(dev->input->info)  {
      if(dev->input->info->maxInputChannels>1) {  
        readBytes = dev->input->pipe->write(len,inputBuffer);
	  }
      else {
        rBuf = malloc(len);
        n=0;
        for(i=0;i<(len/sizeof(PA_SAMPLE_TYPE))/2;i++) {
          ((float *)rBuf)[n]=((float *)inputBuffer)[i];
          ((float *)rBuf)[n+1]=((float *)inputBuffer)[i];
          n+=2;
        }
        readBytes = dev->input->pipe->write(len,rBuf);
        free(rBuf);
      }
	  if(readBytes <= 0) memset(inputBuffer,0,len);
	}
  }
  if(outputBuffer != NULL) { /* handle output to soundcard */
	if(dev->output->info) {
	  if(dev->output->info->maxOutputChannels>1) {
	     readBytes = dev->output->pipe->read(len,outputBuffer);
	  }
      else {
	     rBuf = malloc(len);
		 readBytes = dev->output->pipe->read(len,rBuf);
		 n=0;
         for(i=0;i<(len/sizeof(PA_SAMPLE_TYPE))/2;i++) {
           ((float *)outputBuffer)[n]=((float *)rBuf)[i];
           ((float *)outputBuffer)[n+1]=((float *)rBuf)[i];
		   n+=2;
         }
		 free(rBuf);
	  }
	  if(readBytes <= 0) memset(outputBuffer,0,len);
    }
  }
  return 0;
}

bool SoundDevice::input(bool state) {
  bool res = false;
  if(jack) return true;
  if(!res) res = pa_open(state,PaInput);
  return res;
}

PaError SoundDevice::pa_real_open(int mode) {
  return Pa_OpenStream( ((mode & PaInput) == PaInput)?&input_device.stream:&output_device.stream,
    ((mode & PaInput) == PaInput)?input_device.id:paNoDevice,
    ((mode & PaInput) == PaInput)?(input_device.info->maxInputChannels>1?2:1):0,
    PA_SAMPLE_TYPE,
    NULL,
    ((mode & PaOutput) == PaOutput)?output_device.id:paNoDevice,
    ((mode & PaOutput) == PaOutput)?(output_device.info->maxOutputChannels>1?2:1):0,
    PA_SAMPLE_TYPE,
    NULL,
    SAMPLE_RATE,
    FRAMES_PER_BUFFER,
    0,  /* number of buffers, if zero then use default minimum */
    0, // paClipOff,     /* we won't output out of range samples so don't bother clipping them */
    pa_process,
    &pa_dev );
}

bool SoundDevice::pa_open(bool state,int mode) {
  PaDevInfo *dev,*other;
  int creq,oreq;
  char dir[7];
  
 // PaDeviceInfo *k=NULL;
 // int i=0;
 // do {
//	k=Pa_GetDeviceInfo(i);
//	i++;
///	if(k) printf("KKK %d - %s \n",i,k->name);
//  } while (k!=NULL);
  
  if(mode == PaInput) { // input requested
    dev = &input_device;
    other = &output_device;
    creq = PaInput;
    oreq = PaOutput;
    strcpy(dir,"input");
    dev->id = Pa_GetDefaultInputDeviceID();
  }
  else if(mode == PaOutput) { // output requested
    dev = &output_device;
    other = &input_device;
    creq = PaOutput;
    oreq = PaInput;
    strcpy(dir,"output");
    dev->id = Pa_GetDefaultOutputDeviceID();
  }
  if(state && ((pa_mode & creq) != creq)) {
    dev->info = (PaDeviceInfo*)Pa_GetDeviceInfo( dev->id );
    if(dev->info) notice("Opening %s device: %s",dir,dev->info->name);
    else {
      error("%s device not available",dir);
      return false;
    }
    if((pa_mode & oreq) == oreq) {
      /* input device is already opened...check if we are trying to open the same device */
      if(other->info) { 
		  Pa_StopStream( other->stream );
          Pa_CloseStream( other->stream );
          err = pa_real_open(PaInput|PaOutput);
          if(err == paNoError ) output_device.stream = input_device.stream;
	  }
      else {
        error("Full duplex has been requested but we don't have portaudio information");
        return false;
      }
    }
    else {
      err = pa_real_open(mode);
    }
    if( err != paNoError) {
      Pa_Terminate();
      error("error opening %s sound device: %s",dir,Pa_GetErrorText( err ) );
      return false;
    }
    else {
      err = Pa_StartStream(dev->stream);
      if(err != paNoError) {
         error("error starting %s audio stream: %s",dir,Pa_GetErrorText( err ) );
         return false;
      }
      pa_mode = pa_mode | creq;
    }
  } else if(!state && dev->stream) { // XXX - i have to check if this is still right
     if(dev->info) notice("Closing %s device: %s",dir,dev->info->name);
    if((pa_mode & creq) == creq) {
       if((pa_mode & oreq) == oreq) {
         pa_mode = oreq;
       }
       else {
         Pa_StopStream(dev->stream);
         Pa_CloseStream(dev->stream);
         pa_mode = PaNull;
       }
    }
    dev->stream = NULL;
    dev->info = NULL;
    dev->pipe->flush();
	//delete dev->pipe;
  }
  return true;
}

bool SoundDevice::output(bool state) {
  bool res = false;
  if(jack) return true;
  if(!res) res = pa_open(state,PaOutput);
  return res;
}

bool SoundDevice::open(bool read, bool write) {

  //  notice("detecting sound device");

#ifdef HAVE_JACK
  // we try to open up a jack client
  jack_sample_size = sizeof(jack_default_audio_sample_t);
  if(!jack) // check if it is not allready on
    if( (jack_client = jack_client_new("MuSE")) !=0 ) {
      notice("jack audio daemon detected");
      act("hooking in/out sound channels");
      warning("this feature is still experimental and won't work!");
      warning("you need to stop jack and free the audio card");
      jack = true;
      jack_samplerate = jack_get_sample_rate(jack_client);
      jack_set_process_callback(jack_client, dev_jack_process, this);    
      jack_on_shutdown(jack_client,dev_jack_shutdown,this);

      jack_in_pipe = new Pipe();
      jack_in_pipe->set_output_type("copy_float_to_int16");
      jack_in_pipe->set_block(false,true);

      jack_out_pipe = new Pipe();
      jack_out_pipe->set_input_type("copy_int16_to_float");
      jack_in_pipe->set_block(true,false);

      // open the jack input channel
      jack_in_port = jack_port_register(jack_client, "capture",
					JACK_DEFAULT_AUDIO_TYPE,
					JackPortIsInput, 0);
      // open the jack output channel
      jack_out_port = jack_port_register(jack_client, "playback",
					 JACK_DEFAULT_AUDIO_TYPE,
					 JackPortIsOutput, 0);
      
      jack_activate(jack_client);
      return true;
    }
#endif
  if( ! output(write) ) return false;
  
  //if( ! input(read) ) return false;
  
  return true;
}

void SoundDevice::close() {
  if((pa_mode&PaInput) == PaInput) {
    if((pa_mode&PaOutput) == PaOutput) {
      pa_mode = PaOutput;
      if(output_device.stream == input_device.stream)
	output_device.stream = NULL;
    }
    else pa_mode = PaNull;
    if(input_device.stream) {
      Pa_StopStream( input_device.stream);
      Pa_CloseStream( input_device.stream );
      input_device.stream = NULL;
    }
    input_device.pipe->flush();
    //delete input_device.pipe;
  }
  
  if((pa_mode&PaOutput) == PaOutput) {
    if(output_device.stream) {
      Pa_StopStream( output_device.stream);
      Pa_CloseStream( output_device.stream );
      output_device.stream = NULL;
    }
    output_device.pipe->flush();
    if((pa_mode&PaInput) == PaInput)
      pa_mode = PaInput;
    else pa_mode = PaNull;
  }
  //delete output_device.pipe;
}

int SoundDevice::read(void *buf, int len) {
  // len is in samples: 4*2 32bit stereo
  int res = -1;

  if(jack) {

    res = jack_in_pipe->read(len*2,buf);

  } else if(input_device.stream) { // portaudio

    // takes number of left and right frames (stereo / 2)
    res = input_device.pipe->read(len*2,buf);
  }  
  return res;
}

int SoundDevice::write(void *buf, int len) {
  // len is in samples, for bytes must *2 (16bit)
  int res = -1;

  if(jack) { // jack audio daemon

    res = jack_out_pipe->write(len*2,buf);

  } else if(output_device.stream) { // portaudio
	res = output_device.pipe->write(len,buf);
	//func("dspout available pipe space: %d \n",output_device.pipe->space());
  }
  return res;
}

void SoundDevice::flush_output() {
  if(jack)
    jack_out_pipe->flush();
  else if(output_device.stream) { // portaudio
    output_device.pipe->flush();
  }
}
void SoundDevice::flush_input() {
  if(jack)
    jack_in_pipe->flush();
  else if(input_device.stream) {
   input_device.pipe->flush();
  }
}
