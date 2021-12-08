# Makefile

CC = gcc 
CPPFLAGS = -MMD
CFLAGS = -Wall -Wextra -std=c99 -O0 -g
LDFLAGS = -lm
LDLIBS =

OBJ = test_save_Xor.o main.o
DEP = ${OBJ:.o=.d}

all: main

main: ${OBJ}

clean:
	${RM} ${OBJ} ${DEP} biasH.bias biasO.bias weightHO.weight weightIH.weight main

-include ${DEP}

# END