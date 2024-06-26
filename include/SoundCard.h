#ifndef AUDIO_H
#define AUDIO_H

#include <cmath>
#include <iostream>
#include <SDL2/SDL.h>

#include "Constants.h"

// adapted from https://blog.fredrb.com/2023/08/08/audio-programming-note-sdl/ and turned into C++ class

void oscillator_callback(void *userdata, Uint8 *stream, int len); // SDL requires a function to provide it audio samples if it's buffer runs out

class SoundCard{ // class to provide audio samples to SDL buffer. Currently only produces a sine wave of a single frequency
private:
  float current_step; // current time of waveform
  float step_size; // time advancement to set proper frequency
  float volume; // volume
  SDL_AudioDeviceID audio_device_id; // ID of audio device to play/pause
public:
  bool debug;
  SoundCard(float rate=A4_OSC, float vol=INIT_VOLUME, bool debug=false);
  ~SoundCard();

  float next(); // get the next sample in the sequence
  void set_pause(int pause);
};

#endif