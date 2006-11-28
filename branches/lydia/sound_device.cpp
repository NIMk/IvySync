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

#include <sound_device.h>
#include <utils.h>

/* settings (take care!) */
#define IN_DATATYPE int16_t
#define OUT_DATATYPE int16_t
#define MIX_CHUNK 1152 //2048
#define IN_CHUNK MIX_CHUNK
#define IN_PIPESIZE IN_CHUNK*(sizeof(IN_DATATYPE))*64
#define SAMPLE_RATE 44100 // 44100

#define PA_SAMPLE_TYPE paFloat32
#define PA_SAMPLES_PER_FRAME 2
#define PA_NUM_SECONDS 5
#define FRAMES_PER_BUFFER   (64)
#define PA_PIPE_SIZE MIX_CHUNK*sizeof(PA_SAMPLE_TYPE)*64 

#define INPUT_DEVICE  Pa_GetDefaultInputDeviceID()
#define OUTPUT_DEVICE Pa_GetDefaultOutputDeviceID()


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
}

SoundDevice::~SoundDevice() {
close();
}

static int pa_process( void *inputBuffer, void *outputBuffer, 
	unsigned long framesPerBuffer, 
	PaTimestamp outTime, void *userData )
{
  unsigned int i,n;
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
	for(i=0; i < (len / sizeof(PA_SAMPLE_TYPE)) /2 ;i++) {
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
    if(dev->info) N("Opening %s device: %s",dir,dev->info->name);
    else {
      E("%s device not available",dir);
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
        E("Full duplex has been requested but we don't have portaudio information");
        return false;
      }
    }
    else {
      err = pa_real_open(mode);
    }
    if( err != paNoError) {
      Pa_Terminate();
      E("error opening %s sound device: %s",dir,Pa_GetErrorText( err ) );
      return false;
    }
    else {
      err = Pa_StartStream(dev->stream);
      if(err != paNoError) {
         E("error starting %s audio stream: %s",dir,Pa_GetErrorText( err ) );
         return false;
      }
      pa_mode = pa_mode | creq;
    }
  } else if(!state && dev->stream) { // XXX - i have to check if this is still right
     if(dev->info) N("Closing %s device: %s",dir,dev->info->name);
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
  if(!res) res = pa_open(state,PaOutput);
  return res;
}

bool SoundDevice::open(bool read, bool write) {

  N("open sound device");

  if( ! output(write) ) return false;
  
  if( ! input(read) ) return false;
  
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

  // takes number of left and right frames (stereo / 2)
  res = input_device.pipe->read(len*2,buf);

  return res;
}

int SoundDevice::write(void *buf, int len) {
  // len is in samples, for bytes must *2 (16bit)
  int res = -1;

  res = output_device.pipe->write(len,buf);
  //func("dspout available pipe space: %d \n",output_device.pipe->space());

  return res;
}

void SoundDevice::flush_output() {
  output_device.pipe->flush();
}
void SoundDevice::flush_input() {
   input_device.pipe->flush();
}
