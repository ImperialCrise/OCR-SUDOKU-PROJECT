#!/bin/sh

gcc -Wall -Wextra -Werror -std=c99 -O1 -o main *.c `sdl2-config --cflags --libs` -lSDL2_image

./main
