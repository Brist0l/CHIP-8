# CHIP-8 Emulator

A CHIP-8 emulator written from scratch in C with a (barely) debugger.

![debugger](https://github.com/Brist0l/CHIP-8/blob/main/imgs/debugger.png)

Runs classic games written in original chip-8 syntax: 

Pong, Tetris, Space Invaders, Puzzle, Brick Breaker, etc.

![games](https://github.com/Brist0l/CHIP-8/blob/main/imgs/games.png)


## Features

1) Implements the full CHIP-8 instruction set (35 official opcodes)
2) Accurate sprite rendering and collision detection (VF flag)
3) Configurable key mapping matching the original hex keypad layout
4) Supports timers (delay + sound) 

## Build & Run

Make sure you have SDL3 installed.

`gcc chip_8.c debug.c display.c -l SDL3 -o chip_8`

To run the game set the first argument to be anything other than "1000" , if 
it's "1000" then the program will be loaded from `_fillopcode()` from `debug.c`.

## Project structure
```
  chip8.c      # Instruction decoding + execution cycle
  display.c    # SDL3 display handling
  debug.c      # Handles all of the deubbging stuff

```

## From F to CHIP-8

The first font ever rendered on my emulator:

![font](https://github.com/Brist0l/CHIP-8/blob/main/imgs/f.webp)

The first rudimentary test of any CHIP-8 emulator:

![ibm](https://github.com/Brist0l/CHIP-8/blob/main/imgs/ibm.png)

The first game to be run on my emulator:

![tetris](https://github.com/Brist0l/CHIP-8/blob/main/imgs/tetris.png)

