# Project 2, class OS

> Program: _projeto.cpp_
> 
> Version: 1.1

This project was idealized in OS classes. This is a program that simulate three robots with position got it by an camera (in this case, it's used random numbers) and then, the robots itself should move according with that. 

The goal is to show how much one thread interferes with the anothers, as so, simulate an robot that it's used in competitions.

## Overview

<img title="" src="file:///home/ppcamp/gitRepos/robotsWithThreads/doc/program_running.png" alt="program_running.png" data-align="center">

## Summary:

1. [Compiling](#Compiling)
2. [Commons features](#Commons)
3. [Program Infos](#Infos)
4. [PDF](#Pdf)
5. [Functions implemented](#Functions)
6. [Comments](#Comments)

---

## Compiling

1. To compile only useful (_working_) file:
   
   ```bash
   CFLAGS="-std=c++11 $(pkg-config --cflags --libs sdl2)"
   
   # Compile
   g++ projeto.cpp -o p -lpthread $(echo $CFLAGS)
   # Those libs need to be in last position, otherwise, the compiler will not link
   # Execute
   ./p
   ```
   
   > The flag -g is used in debug mode

---

## Commons

### Positions on vectors

1. #### Camera
   
   > The camera corresponds to pos **0**, in vectors

2. #### GPS
   
   > The gps corresponds to pos **1**, in vectors

3. #### Odometro
   
   > The odometro corresponds to  pos **2**, in vectors

4. #### Used only in color struct
   
   > Corresponds to color of a 'blank screen'
   
   ### Macros:

5. #### ROBOTS_AMOUNT
   
   ```cpp
    #define ROBOTS_AMOUNT 3
   ```
   
   > Corresponds to how much robots do we have

6. #### SOURCES_AMOUNT
   
   ```cpp
    #define SOURCES_AMOUNT ROBOTS_AMOUNT
   ```
   
   > Corresponds to how much sources do we have

7. #### nullptr
   
   ```cpp
    #define nullptr NULL
   ```

8. #### error_on_initialize()
   
   ```cpp
        #define error_on_initialize() \
            { printf("Error on initialize. Code: %s\n", SDL_GetError()); }
   ```
   
   > Used in sdl2 errors

### Consts:

1. #### Boards (screen)
   
   ```cpp
    const int BOARD_Y = 300;
    const int BOARD_X = 400;
   ```

2. #### Time in seconds
   
   ```cpp
    const int TIME_MIN = 0; // in seconds
    const int TIME_MAX = 4;
    const int TIME_EQUIPAMENTS[ROBOTS_AMOUNT] = {TIME_MIN, TIME_MIN, TIME_MIN}; // Remember the order in positions...
   ```

3. #### Buffer size limit
   
   ```cpp
    const int BUFFER_SIZE_MAX = 100;
   ```

4. #### Iterator needed to pass id to each thread
   
   ```cpp
    int aux_iterator_amount[3] = {0, 1, 2};
   ```
   
   > Note that the 3 it's because the set up value in macro _ROBOTS_AMOUNT_

5. #### Screen size
   
   ```cpp
    const int SCREEN_WIDTH = BOARD_X + ROBOT_HEIGHT,
          SCREEN_HEIGHT = BOARD_Y + ROBOT_WIDTH;
   ```
   
   > This is only used in sdl function

6. #### Robot sizes
   
   ```cpp
    const int ROBOT_HEIGHT = 20, ROBOT_WIDTH = 20;
   ```

### Structs:

1. #### Object
   
   ```cpp
    struct object {
        int x, y, robot, source;
        object(int x, int y, int robot, int source) {
            this->x = x;
            this->y = y;
            this->robot = robot;
            this->source = source;
        }
    };
   ```
   
   > Used in buffer.

2. #### _Unnamed_
   
   ```cpp
    struct {
        bool mark_equipament[SOURCES_AMOUNT];
        int position[SOURCES_AMOUNT][2];
    } robots[ROBOTS_AMOUNT];
   ```
   
   > Used to storage robots before draw them.

3. #### _Unnamed_
   
   ```cpp
    struct {
    Uint8 r, g, b, a;
    } colors[ROBOTS_AMOUNT + 1] = {
        {37, 85, 179, 1}, {148, 71, 194, 1}, {49, 106, 79, 1}, {30, 30, 30, 1}};
   ```
   
   > Store robots and board(last one) colors.

---

## Infos

Compiler Version:

* g++ 11 + (11, or newest)

Libs needed:

```cpp
#include <SDL2/SDL.h>   // sdl2
#include <pthread.h>    // threads
#include <semaphore.h>  // semaphore
#include <unistd.h>     // sleep,
#include <cstdlib>      // rand
#include <ctime>        // seed,
#include <queue>        // buffer
```

> The package ***SDL2-Version:2.0.9*** can be found: in ubuntu repo as 
> 
> ```shell
> sudo apt-get install libsdl2-dev
> # the command below show the flags of gcc
> sdl2-config --cflags --libs
> ```
> 
>  or [here](https://www.libsdl.org/download-2.0.php)

License and versions:

* Authors: [ . . . ]

* Version: _1.0.1_

* License: _GPL-3.0_

---

## Pdf

Instructions for program: [doc/robotica.pdf](/doc/robotica.pdf)

---

## Functions

Funcs that are used in thread (working):

- [x] source_rand

- [x] make_robots

- [x] set_to_draw

- [x] draw
  
  > In version 1.0, draw function shows on terminal. In version 1.1+ shows in window.

---

## Comments

##### @ppcamp:

> To debug the threads you can use ***GBD***:
> 
> > Remember that, to debug your code, you need to use the ***-g*** **flag** when compiling.

---

##### @ppcamp:

> Useful commands to ***GDB***:
> 
> * gdb a.out -- Start _GDB_;
> 
> * run -- Execute the runnable file _'a.out'_;
> 
> * backtrace -- Show last executions (frames);
> 
> * frame *N* -- Select frame _N_;
> 
> * print *var* -- Print some variable (_**e.g.**_, from frame);
> 
> * kill -- Kill program running;
> 
> * break *N* -- BreakPoint. Showed in frame too;
> 
> * next -- Continue execution
>   
>   > See more on: [_Unknow Road_][gdb]

---

##### @ppcamp:

> GDB commands to loop until get error
> 
> ```gdb
> set $n = 100
> while $n-- > 0
>   printf "\n\n\n\t\tStarting program\n"
>   run
>   if $_siginfo
>     printf "\n\n\n\t\tReceived signal %d, stopping\n", $_siginfo.si_signo
>     loop_break
>   else
>     printf "\n\n\n\t\tProgram exited with Success!\n"
>     end
> end
> ```

---

##### @ppcamp:

> Debug functions:
> 
> ```cpp
> void debug_buffer() {
>  while (!buffer.empty()) {
>    printf("Robot: %d, Source: %d, Pos: (%d,%d)\n", buffer.front().robot,
>           buffer.front().source, buffer.front().x, buffer.front().y);
>    buffer.pop();
>  }
> }
> ```
> 
> ```cpp
> void debug_robots() {
>  for (int i = 0; i < ROBOTS_AMOUNT; i++) {
>    printf("Robot[%d]:\n", i);
>    printf("\tSource mark: {%d,%d,%d}\n", robots[i].mark_equipament[0],
>           robots[i].mark_equipament[1], robots[i].mark_equipament[2]);
>    printf("\tPosition:\n");
>    printf("\t\tSource [0]: (%d,%d)\n", robots[i].position[0][0],
>           robots[i].position[0][1]);
>    printf("\t\tSource [1]: (%d,%d)\n", robots[i].position[1][0],
>           robots[i].position[1][1]);
>    printf("\t\tSource [2]: (%d,%d)\n", robots[i].position[2][0],
>           robots[i].position[2][1]);
>  }
> }
> ```
> 
> ```cpp
> void debug_robots_to_draw() {
>  for (int i = 0; i < ROBOTS_AMOUNT; i++) {
>    printf("Draw robot %d:\n", i);
>    printf("\tPos: (%d,%d)\n", robots_to_draw[i].x, robots_to_draw[i].y);
>  }
> }
> ```

---

##### @ppcamp:

> To compile ***SDL2*** on linux:
> 
> ```sh
> ./configure
> make
> sudo make install
> ```

---

##### @ppcamp:

> Some ***SDL2*** tutorials links:
> 
> - [LazyFoo][lazy]
> - [Sonar][sonar]

---

[sonar]:https://www.youtube.com/watch?v=uvxV1xlApm4&list=PLRtjMdoYXLf75jCBW6Ln2aS7DZoB5q2Hr
[lazy]:http://lazyfoo.net/tutorials/SDL/01_hello_SDL/linux/cli/index.php
[sdl2]:https://www.libsdl.org/download-2.0.php
[gdb]:http://www.unknownroad.com/rtfm/gdbtut/gdbsegfault.html