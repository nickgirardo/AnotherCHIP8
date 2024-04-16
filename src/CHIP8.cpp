#include "CHIP8.h"
#include <fstream>
#include <vector>
#include <bitset>
#include <string>
#include <iterator>
#include <sstream> 
#include <arpa/inet.h>

CHIP8::CHIP8(bool visible){
    Display disp(visible);
}

void CHIP8::load(std::string filename){
    std::ifstream program(filename,std::ios::binary);
    if ( !(program.is_open()) ) {
        std::cout << "File " << __FILE__ << " Line " << __LINE__ << " " << std::endl;
        std::string err_msg = filename+" couldn't be opened!";
        throw std::runtime_error(err_msg);
    }
    program.unsetf(std::ios::skipws);
// grab length of file, in units of unsigned chars (ie. 1 byte)
    program.seekg (0, std::ios::end);
    int length = program.tellg();
    program.seekg (0, std::ios::beg);
    if(length > MAX_PROGRAM_SIZE){
        program.close();
        std::cout << "File " << __FILE__ << " Line " << __LINE__ << " " << std::endl;
        std::string err_msg = filename+" is too big!";
        throw std::runtime_error(err_msg);
    }
// CHIP-8 Programs must align with even addresses
    if(length%2!=0){
        program.close();
        std::cout << "File " << __FILE__ << " Line " << __LINE__ << " " << std::endl;
        std::string err_msg = filename+" is misaligned!";
        throw std::runtime_error(err_msg);
    }
// Assuming you got here, the program fits in the RAM of the CHIP-8. Copy the data into the temp buffer, then copy over to system
    std::vector<uint8_t> buffer;
    buffer.reserve(length);
    buffer.insert(buffer.begin(),std::istream_iterator<uint8_t>(program),std::istream_iterator<uint8_t>());
// iterate by two since we read two bytes at a time
    for(int i=0; i< length; i+=2){
        uint8_t first, second;
        if(is_big_endian()){
            first = buffer[i];
            second = buffer[i+1];
        }
        else{
            first = buffer[i+1];
            second = buffer[i];
        }
        this->mem.write(START+i, first);
        this->mem.write(START+i+1, second);
    }
    program.close();
    loaded = true;
}

void CHIP8::print() const{
    std::cout << "CPU" << std::endl;
    cpu.print();
    std::cout << "Display" << std::endl;
    disp.print();
    std::cout << "Memory" << std::endl;
    mem.dump();
}

std::string CHIP8::disassemble() const{
    std::stringstream output;
    for(int addr=START; addr<MAX_PROGRAM_SIZE; addr+=2){
        auto instruction = this->mem.read_instruction(addr);
        if(instruction!=0){
            output << std::hex << addr << " " << instruction << '\n' << std::dec;
        }
    }
    return output.str();
}

uint16_t CHIP8::fetch() const{
    if(loaded){
        return this->mem.read_instruction(this->cpu.get_pc());
    }
    throw std::invalid_argument("No program loaded");
}

Instruction CHIP8::decode(uint16_t machine_code) const{
    return Instruction(machine_code);
}


bool is_big_endian(void){
    union {
        uint32_t i;
        char c[4];
    } bint = {0x01020304};

    return bint.c[0] == 1;
}