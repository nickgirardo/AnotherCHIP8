#include "CPU.h"
#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>

CPU::CPU(bool threading_on){
  pc=START;
  sound = 0;
  delay=0;
  for(int i=0; i< 16; ++i){
    Vx[i] = 0;
  }
  I=0;
  SDL_AudioSpec desired_spec = {
      .freq = SAMPLE_RATE,
      .format = AUDIO_F32,
      .channels = 1,
      .samples = BUFFER_SIZE,
      .callback = oscillator_callback,
      .userdata = this,
  };
  SDL_AudioSpec obtained_spec;
  if(SDL_InitSubSystem(SDL_INIT_AUDIO) != 0){
      std::cout <<  SDL_GetError() << std::endl;
      throw std::invalid_argument("SDL_Init failed");
  }
  audio_device = SDL_OpenAudioDevice(NULL, 0, &desired_spec, &obtained_spec, SDL_AUDIO_ALLOW_ANY_CHANGE);
  if(audio_device==0){
      std::string message = "AHHHHH! Failed to open Audio Device: "+ std::string(SDL_GetError());
      throw std::invalid_argument(message);
  }
  SDL_PauseAudioDevice(audio_device,0);     
  if(threading_on){
    std::cout << "Threading On" << std::endl;
  // Create and detach deamon thread that auto-decrements delay counter if non-zero
    std::thread delay_dec( [this] { this->decrement_delay(); } );
    delay_dec.detach();
  // same for sound
    std::thread sound_dec( [this] { this->decrement_sound(); } );
    sound_dec.detach();
  }
}

CPU::~CPU(){
    SDL_CloseAudioDevice(audio_device);
}

uint16_t CPU::get_pc() const{return this->pc;}

uint8_t CPU::get_sound() const{
  return this->sound.load();
}

uint8_t CPU::get_delay() const{
  return this->delay;
}

uint8_t CPU::get_Vx(uint8_t i) const{
  if(i>=16){
    throw std::invalid_argument("Invalid Register");
  }
  return this->Vx[i];
}

uint8_t CPU::get_VF() const{
  return this->Vx[15];  
}

uint16_t CPU::get_I() const{return this->I&&I_MASK;}

void CPU::set_sound(uint8_t value){
  this->sound.store(value);
}

void CPU::set_delay(uint8_t value){
  this->delay.store(value);
}

void CPU::decrement_delay(){
  thread_local auto last_update = std::chrono::steady_clock::now();
  auto period = get_clock_period();
  while(1){
// If you are already at 0, don't do anything
    if(this->get_delay()==0){
      last_update = std::chrono::steady_clock::now();
      continue;
    }
// Counter is non-zero. Check if elapsed time is greater than 
    auto now = std::chrono::steady_clock::now();
    auto delta = std::chrono::duration_cast<std::chrono::microseconds> (now-last_update);
    if(delta.count()>period){
      delay -= 1;
      last_update = std::chrono::steady_clock::now();
    }
  }
}

void CPU::decrement_sound(){ // identical to decrement delay
  thread_local auto last_update = std::chrono::steady_clock::now();
  while(1){
// If you are already at 0, check if you should be playing sound
    if(this->get_sound()==0){
      SDL_PauseAudioDevice(audio_device,1);
      last_update = std::chrono::steady_clock::now();
      continue;
    }
    else{
      SDL_PauseAudioDevice(audio_device,0);
    }
// Counter is non-zero. Check if elapsed time is greater than 
    auto now = std::chrono::steady_clock::now();
    auto delta = std::chrono::duration_cast<std::chrono::microseconds> (now-last_update);
    if(delta.count()>get_clock_period()){
      sound -= 1;
      last_update = std::chrono::steady_clock::now();
    }
  }
}

void CPU::print() const{
  std::cout << "PC: " << std::hex << std::showbase <<  get_pc() << std::endl;
  std::cout << "Sound: " << static_cast<unsigned int>(get_sound()) << std::endl;
  std::cout << "Delay: " << static_cast<unsigned int>(get_delay()) << std::endl;
  std::cout << "I: " << get_I() << std::endl;
  for(int i=0; i< 16; ++i){
    std::cout << "V" << std::hex << std::uppercase << i << " :" << std::dec<<  static_cast<unsigned int>(get_Vx(i)) << std::endl;
  }
  std::cout << audio_device << std::endl;
}

void oscillator_callback(void *userdata, Uint8 *stream, int len) {
  float *fstream = (float *)stream;
  for (int i = 0; i < BUFFER_SIZE; i++) {
    float v = (static_cast<CPU*>(userdata))->osc->next();
    fstream[i] = v;
  }
}

void CPU::set_VF(bool is_set){
  this->Vx[15] = is_set;
}

void CPU::set_Vx(uint8_t i,uint8_t value) {  
  if(i>=15){
    throw std::invalid_argument("invalid register (Also, cant write to F register)");
  }
  this->Vx[i] = value;
}

void CPU::set_I(uint16_t value) {
  this->I = value & I_MASK; //  mask out top nibble of the input value
}

void CPU::increment_pc(){
  this->pc = this->pc + instruction_size;
}

void CPU::decrement_pc(){
  this->pc = this->pc - instruction_size;
}

void CPU::set_pc(uint16_t value){
  this->pc = value;
}

void CPU::push_stack(uint16_t value){
  if(chip_stack.size() == max_stack_size){
    throw std::invalid_argument("Stack overflow");
  }
  this->chip_stack.push_front(value);
}

uint16_t CPU::pop_stack(){
  auto output = chip_stack.at(0);
  this->chip_stack.pop_front();
  return output;
}

uint16_t CPU::poke_stack(uint8_t value){
// debugging function to check values in the stack without desturbing it
  uint16_t output;
  try{
    output =   this->chip_stack.at(value);
  }
  catch(const std::out_of_range& e){
    std::cout << e.what() << std::endl;
    return 0; // PC should never ever end up in this state normally
  }
  return output;
}

void CPU::reset(){
  pc=START;
  sound = 0;
  delay=0;
  for(int i=0; i< 16; ++i){
    Vx[i] = 0;
  }
  I=0;
  SDL_PauseAudioDevice(audio_device,1);
  this->set_delay(0);
  this->set_sound(0);
}