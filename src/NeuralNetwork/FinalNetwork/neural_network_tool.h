#ifndef NEURAL_NETWORK_TOOLS_H
#define NEURAL_NETWORK_TOOLS_H

# include <stdlib.h>
# include <stdio.h>
# include <err.h>
#include <math.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

double Random();

double Sigmoid(double x);

double Derivative_Sigmoid(double x);

struct Data_test{
  double input[28*28];
  double goal[10];
};



void init_sdl2();

SDL_Surface* load_image2(char *path);

Uint8* pixel_ref2(SDL_Surface *surf, unsigned x, unsigned y);


#endif
