#include <fstream>
#include <cstdio>
#include <cassert>
#include <SDL2/SDL.h>
#include <stack>
using namespace std;

class Graphics {
  public:

  SDL_Window* window;
  SDL_Renderer* renderer;
  SDL_Surface* surface;
  SDL_Texture* texture;

  void init();
  void render();

};

void Graphics::init(){
  SDL_CreateWindowAndRenderer(64, 32, SDL_WINDOW_RESIZABLE, &window, &renderer);
}
void Graphics::render(){


}

class Chip8 {
  public:

    unsigned char memory [4096];
    unsigned char display [64*32] = {0};
    
    unsigned short pc;
    unsigned short I;

    unsigned char delay_timer, sound_timer;
    unsigned char v[16];

    unsigned short op_code;
    std::stack<unsigned short> address_stack;
    unsigned short sp;

    //BASIC: font
    //BASIC: input support
    //BASIC: timers

    int init();
    void emulate();
    void show_screen();
};

void Chip8::show_screen (){
  printf("DISPLAY.\n");
  for (int i = 0; i < 32; i++){
    for (int j = 0; j < 64; j++){
      if(display[i*64+j]){
        //printf("%x", display[i*64+j]);
        printf("1");
      }
      else
        printf(".");
    }
    printf("\n");
  }

}

//TO-DO: should pass filename str
int Chip8::init (){
  op_code = I = sp = 0;
  pc = 0x0200;

  streampos size;
  char * memblock;

  ifstream rom ("./IBM_Logo.ch8", ios::in|ios::binary|ios::ate);
  if (rom.is_open()){

    size = rom.tellg();
    memblock = new char [size];
    rom.seekg (0, ios::beg);
    rom.read (memblock, size);
    rom.close();
    
    for (int i = 0; i < size; i++){
      memory[0x0200+i] = memblock[i] & 0x000000FF;
      //printf("%x ", memory[0x0200+i]);
    }
    delete[] memblock; 
  }
  return size;
}

void Chip8::emulate (){
  unsigned char X, Y, N, NN;
  unsigned short NNN;

  op_code = (memory[pc] << 8) | memory[pc+1];
  //printf("pc: %d\n", pc);
  //printf("op_code: %x\n", op_code);
  pc = pc + 2;

  X = op_code & 0x0F00;
  Y = op_code & 0x00F0;
  N = op_code & 0x000F;
  //TO-DO: bitshift and bitwise OR existing variables 
  NN = op_code & 0x00FF;
  NNN = op_code & 0x0FFF;

  switch (op_code & 0xF000){
    case 0x0000:
      switch (op_code & 0x000F){
        case 0x0000:
          for (int px = 0; px < 64*32; px++){
            if (display[px] == 1) 
              display[px] = 0;
          }
          // TEST
          show_screen();
          break;
        case 0x000E:
          break; 
      }
      break;
    case 0x1000: 
      pc = NNN;
      //TEST
      //assert(pc==NNN);
      break;
    case 0x6000:
      v[X] = NN;
      // TEST 
      //assert(v[X] == NN);
      break;
    case 0x7000:
      v[X] = (v[X]+NN) & 0x0100;
      break;
    case 0xA000:
      I = NNN;
      //TEST
      //assert(I==NNN);
      break;
    case 0xD000:
     unsigned char x = v[X] & 64;
     unsigned char y = v[Y] & 32;
     //idk why we're setting carry flag to 0 
     v[N] = 0;

     for (int i = 0; i < N; i++){
       if (y+i > 32)
         continue;
       unsigned char pixels = memory[I + i];
       for (int j = 0; j < 8; j++){
         if (x+j > 64)
           continue;
         display[(y+i)*64+x+j] ^= (pixels & (1 << j));
         v[N] = !(display[(y+i)*64+x+j]);
       }
     }
     show_screen();
     break;
  }
}

int main(){
  Chip8 myChip8;
  Graphics graphics;
  int rom_length = myChip8.init();

  //main loop 
  for (int i = 0; i < rom_length; i++){
    myChip8.emulate();
    //if 00E0 or DXYN, update graphics
    //check user input
  }
  return 0;
}
