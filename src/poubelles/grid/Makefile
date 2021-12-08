# Makefile

CC = gcc -fsanitize=address
CPPFLAGS = -MMD
CFLAGS = -Wall -lm -ldl -Wextra -Werror -std=c99 -O3
LDFLAGS =
LDLIBS = -lm `pkg-config --libs sdl SDL_image`

OBJ = grid_detection.o ../list/liste_chainee.o\
 ../tools/tools.o ../tools/pixel_operations.o    main.o
DEP = ${OBJ:.o=.d}

all: main

main: ${OBJ}

clean:
	${RM} ${OBJ} ${DEP} main

-include ${DEP}

# END