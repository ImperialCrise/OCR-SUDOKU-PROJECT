#ifndef LOADER_H
#define LOADER_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <err.h>

#include "../tools/pixel_operations.h"

void toGreyScale(SDL_Surface *image_surface);
void SauvolaThresholding(SDL_Surface *image_surface);

#endif