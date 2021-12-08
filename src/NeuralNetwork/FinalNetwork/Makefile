# Makefile

CC = gcc 
CPPFLAGS= `pkg-config --cflags sdl, pkg-config --libs sdl SDL_image, pkg-config --libs sdl SDL_gfx` -MMD
CFLAGS= -Wall -Wextra -std=c99 -O3
LDFLAGS=
LDLIBS= `pkg-config --libs sdl SDL_image, pkg-config --libs sdl SDL_gfx` -lm

OBJ = neural_network.o neural_network_tool.o main.o
DEP = ${OBJ:.o=.d}

all: main

main: ${OBJ}

clean:
	${RM} ${OBJ} ${DEP} main

-include ${DEP}

# END

