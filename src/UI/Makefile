# Makefile

CC = gcc
CFLAGS = `pkg-config --cflags sdl gtk+-3.0` -Wall -ldl -Wextra 
LDLIBS = `pkg-config --libs sdl SDL_image gtk+-3.0` -lm 

EXE = plain disc state paddles duel main

all: main

main: ../loader/loader.o ../tools/tools.o ../tools/pixel_operations.o ../grid_detection2/grid_detection.o ../buildgrid/SDL_rotozoom.o ../buildgrid/buildgrid.o



.PHONY: clean

clean:
	${RM} $(EXE)

# END