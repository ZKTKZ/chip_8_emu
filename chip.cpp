#include <typeinfo>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <cassert>
#include <experimental/random>
#include <SDL2/SDL.h>
#include <stack>

using std::cerr;
using std::cout;
using std::endl;
using std::ifstream;
using std::streampos;
using std::ios;

class Chip8;

class Graphics {
  public:

  int init();
};

int Graphics::init(){
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0 ){
    cerr << "SDL_Init Error: " << SDL_GetError() << endl;
    return EXIT_FAILURE;
  }

  SDL_Window* window = SDL_CreateWindow("Chip 8", 100, 100, 64, 32, SDL_WINDOW_SHOWN);
  if (window == nullptr){
    cerr << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
    return EXIT_FAILURE;
  }

  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == nullptr){
    cerr << "SDL_CreateRenderer Error" << SDL_GetError() << endl;
    if (window == nullptr){
      SDL_DestroyWindow(window);
    }
    SDL_Quit();
  }
  
  SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);
  if (texture == nullptr){
    cerr << "SDL_CreateTexture Error: " << SDL_GetError() << endl;
    if (renderer != nullptr){
      SDL_DestroyRenderer(renderer);
    }
    if (window != nullptr){
      SDL_DestroyWindow(window);
    }
    SDL_Quit();
    return EXIT_FAILURE;
  }

      for (int i = 0; i < 20; i++) {
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
        SDL_Delay(100);
    }

  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return EXIT_SUCCESS;
}

class Chip8 {
  friend class Graphics;
  public:

    unsigned char memory [4096];
    unsigned char display [64*32] = {0};
    
    unsigned short pc;
    unsigned short I;

    unsigned char delay_timer, sound_timer;
    unsigned char v[16];

    unsigned short op_code;
    //short or char, chck pls
    unsigned short stack[16]; 
    unsigned short sp;

    //uninitialized
    unsigned char X, Y, N, NN;
    unsigned short NNN;

    unsigned char x_0, y_0;
    bool drawFlag = false;
    //Fonts
    unsigned char fonts [16*5] = {
      0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
      0x20, 0x60, 0x20, 0x20, 0x70, // 1
      0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
      0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
      0x90, 0x90, 0xF0, 0x10, 0x10, // 4
      0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
      0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
      0xF0, 0x10, 0x20, 0x40, 0x40, // 7
      0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
      0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
      0xF0, 0x90, 0xF0, 0x90, 0x90, // A
      0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
      0xF0, 0x80, 0x80, 0x80, 0xF0, // C
      0xE0, 0x90, 0x90, 0x90, 0xE0, // D
      0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
      0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    //BASIC: input support

    void init();
    void emulate();
    void show_screen();
};

void Chip8::show_screen (){
  printf("DISPLAY.\n");
  for (int i = 0; i < 32; i++){
    for (int j = 0; j < 64; j++){
      if(display[i*64+j]){
        //printf("%x", display[i*64+j]);
        printf("*");
      }
      else
        printf(".");
    }
    printf("\n");
  }

}

//TO-DO: should pass filename str
void Chip8::init (){
  op_code = I = sp = 0;
  pc = 0x0200;

  streampos size;
  char * memblock;

  ifstream rom ("./ibm.ch8", ios::in|ios::binary|ios::ate);
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
}

void Chip8::emulate (){

    //default:
    //write sprite to memory
    //starting with 0x0051 (81 in base 10)
    //have global counter var which is incremented
    //sprite & mem are both 1 byte (8 bits)

  op_code = (memory[pc] << 8) | memory[pc+1];
  //printf("pc: %d\n", pc);
  //printf("op_code: %x\n", op_code);
  pc = pc + 2;
  drawFlag = false;

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
          drawFlag = true;
          // TEST
          //show_screen();
          break;
        case 0x000E:
          pc = stack[sp-1];
          break; 
      }
      break;
    case 0x1000: 
      pc = NNN;
      //TEST
      //assert(pc==NNN);
      break;
    case 0x2000:
      stack[sp] = pc;
      sp++;
      break;
    case 0x3000:
      if (v[X] == NN)
        pc+=2;
      break;
    case 0x4000:
      if (v[X] != NN)
        pc+=2;
      break;
    case 0x5000:
      if (v[X] == v[Y])
        pc+=2;
      break;
    case 0x6000:
      v[X] = NN;
      // TEST 
      //assert(v[X] == NN);
      break;
    case 0x7000:
      v[X] = (v[X]+NN) & 256;
      break;
    case 0x8000:
      switch(op_code & 0x000F){

        case 0x0000:
          v[X]=v[Y];
          break;
        case 0x0004:
          if (v[X] + v[Y] >= 0x0100){
            v[0x000F] = 1;
          }
          else{
            v[0x000F] = 0;
          }
          v[X] = (v[X] + v[Y]) & 0x0100;
          break;  
        case 0x0005:
          if(v[Y] > v[X])
            v[0x000F] = 0;
          else
            v[0x000F] = 1;
          v[X] -= v[Y];
          break;
        case 0x0007:
          if(v[Y] >= v[X])
            v[0x000F] = 1;
          else
            v[0x000F] = 0;
          v[X] = v[Y] - v[X];
          break;
        case 0x0001:
          v[X] |= v[Y];
          break;
        case 0x0002:
          v[X] &= v[Y];
          break;
        case 0x0003:
          v[X] ^= v[Y];
          break;
        case 0x0006:
          v[0x000F] = v[Y] & (0x100 >> 8);
          v[X] = v[Y] >> 1;
          break;
        case 0x000E:
          v[0x000F] = v[Y] & (1 << 8);
          v[X] = v[Y] << 1;
          break;
      }
      break;
    case 0x9000:
      if (v[X] != v[Y])
        pc+=2;
      break;
    case 0xA000:
      I = NNN;
      //TEST
      //assert(I==NNN);
      break;
    case 0xB000:
      if (NNN+v[0] > 0x0200)
        pc = NNN+v[0];
      break;
    case 0xC000:
      // v[X] = std::experimental::randint(0, 255) & NN;
      break;
    case 0xD000:
     //From Multigesture's sol'n 
     x_0 = v[X >> 8] % 64;
     y_0 = v[Y >> 4] % 32;
     v[0x000F] = 0;

     for (int i = 0; i < N; i++){
       //if (y+i > 32)
         //continue;
       unsigned char pixels = memory[I + i];
       for (int j = 0; j < 8; j++){
         //if (x+j > 64)
           //continue;
         if (pixels & (0x0080 >> j)){
           if (display[(y_0+i)*64+x_0+j] == 1)
             v[0x000F] = 1;
           display[(y_0+i)*64+x_0+j] ^= 1;
         }
       }
     }
     drawFlag = true;
     show_screen();
     break;

  }
  if (delay_timer > 0)
    delay_timer--;
  if (sound_timer > 0)
    sound_timer--;
}

int main(){
  //Chip8 myChip8;
   Graphics graphics;

  // myChip8.init();
  graphics.init();

  //main loop 
  //should be infinite
  // for (int i = 0; i < 150; i++){
  //   myChip8.emulate();
    // if (myChip8->drawFlag){
    // }
    //check user input
  // }
  
  return 0;
}
