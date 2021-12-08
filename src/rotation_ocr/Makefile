# Simple SDL mini Makefile

CC=gcc -fsanitize=address

CPPFLAGS= `pkg-config --cflags sdl` -MMD
CFLAGS= -Wall -Wextra -Werror -std=c99 -O3 
LDFLAGS= -lm
LDLIBS= `pkg-config --libs sdl SDL_image` -lm

all: display

display: display.o  ../tools/pixel_operations.o 
display.o: ../tools/pixel_operations.h

clean:
	${RM} *.o
	${RM} *.d
	${RM} display

# END
