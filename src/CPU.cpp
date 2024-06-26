#include "CPU.h"
#include <iostream>

CPU::CPU(){
  pc=START;
  sound = 0;
  delay=0;
  last_update = std::chrono::steady_clock::now();
  for(int i=0; i< 16; ++i){
    Vx[i] = 0;
  }
  I=0;
}

uint16_t CPU::get_pc() const{return this->pc;}

uint8_t CPU::get_sound() const{
  return this->sound;
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

uint16_t CPU::get_I() const{return this->I&I_MASK;}

std::chrono::steady_clock::time_point CPU::get_last_update(){return last_update;}

void CPU::set_last_update(){this->last_update = std::chrono::steady_clock::now();}

void CPU::set_sound(uint8_t value){
  this->sound = value;
}

void CPU::set_delay(uint8_t value){
  this->delay = value;
}

void CPU::print() const{
  std::cout << "PC: " << std::hex << std::showbase <<  get_pc() << std::endl;
  std::cout << "Sound: " << static_cast<unsigned int>(get_sound()) << std::endl;
  std::cout << "Delay: " << static_cast<unsigned int>(get_delay()) << std::endl;
  std::cout << "I: " << get_I() << std::endl;
  for(int i=0; i< 16; ++i){
    std::cout << "V" << std::hex << std::uppercase << i << " :" << std::dec<<  static_cast<unsigned int>(get_Vx(i)) << std::endl;
  }
}

void CPU::set_VF(bool is_set){
  this->Vx[15] = is_set;
}

void CPU::set_Vx(uint8_t i,uint8_t value) {  
  if(i>15){
    throw std::invalid_argument("invalid register");
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

void CPU::reset(){ // hard reset CPU to known state
  pc=START;
  sound = 0;
  delay=0;
  for(int i=0; i< 16; ++i){
    Vx[i] = 0;
  }
  I=0;
  this->set_delay(0);
  this->set_sound(0);
  last_update = std::chrono::steady_clock::now();
}