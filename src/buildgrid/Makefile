# Makefile

CC = gcc -fsanitize=address
CPPFLAGS = -MMD
CFLAGS = -Wall -lm -ldl -std=c99 -O3
LDFLAGS =
LDLIBS = -lm `pkg-config --libs sdl SDL_image`

OBJ = buildgrid.o ../tools/tools.o \
	 ../tools/pixel_operations.o  SDL_rotozoom.o  main.o
DEP = ${OBJ:.o=.d}

all: main

main: ${OBJ}

clean:
	${RM} ${OBJ} ${DEP} main

-include ${DEP}

# END