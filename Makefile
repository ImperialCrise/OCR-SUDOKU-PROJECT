# Makefile

CC = gcc
CFLAGS = `pkg-config --cflags sdl gtk+-3.0` -Wall -ldl -Wextra 
LDLIBS = `pkg-config --libs sdl SDL_image SDL_ttf gtk+-3.0` -lm 

EXE = ./ocr src/UI/main ./src/loader/loader.o ./src/tools/tools.o ./src/tools/tools.d ./src/tools/pixel_operations.o ./src/tools/pixel_operations.d ./src/grid_detection2/grid_detection.o ./src/grid_detection2/grid_detection.d ./src/grid_detection2/SDL_rotozoom.o ./src/buildgrid/buildgrid.o ./src/buildgrid/SDL_rotozoom.o ./src/NeuralNetwork/FinalNetwork/neural_network.o ./src/NeuralNetwork/FinalNetwork/neural_network.d ./src/NeuralNetwork/FinalNetwork/neural_network_tool.o ./src/NeuralNetwork/FinalNetwork/neural_network_tool.d ./src/solver/solver.o ./src/NeuralNetwork/FinalNetwork/neural_network.o ./src/NeuralNetwork/FinalNetwork/neural_network_tool.o

all: src/UI/main install

src/UI/main: ./src/loader/loader.o  ./src/tools/tools.o ./src/tools/pixel_operations.o ./src/grid_detection2/grid_detection.o ./src/buildgrid/SDL_rotozoom.o ./src/buildgrid/buildgrid.o ./src/solver/solver.o ./src/NeuralNetwork/FinalNetwork/neural_network.o ./src/NeuralNetwork/FinalNetwork/neural_network_tool.o

install:
	mv src/UI/main ./ocr

.PHONY: clean

clean:
	${RM} $(EXE)

# END