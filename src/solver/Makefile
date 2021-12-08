# Makefile

TARGET = solver
CC = gcc -fsanitize=address
CPPFLAGS = -MMD
CFLAGS = -Wall -Wextra -std=c99 -O0 -g
LDFLAGS =
LDLIBS =

OBJ = reader.o solver.o writer.o main.o
DEP = ${OBJ:.o=.d}

all: solver

solver: ${OBJ}

clean:
	${RM} ${OBJ} ${DEP} ${TARGET} 

-include ${DEP}

# END