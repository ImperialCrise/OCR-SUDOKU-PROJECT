# Simple SDL mini Makefile

CC = gcc
CFLAGS = `pkg-config --cflags sdl` -Wall -ldl -Wextra -Werror -std=c99 -O3
LDLIBS = `pkg-config --libs sdl SDL_image` -lm -fsanitize=address


all: display grayscale main

main: loader.o ../tools/pixel_operations.o
grayscale: pixel_operations.o grayscale.o

grayscale.o: pixel_operations.h

clean:
	${RM} *.o
	${RM} *.d
	${RM} display
	${RM} loader
	${RM} grayscale

# END