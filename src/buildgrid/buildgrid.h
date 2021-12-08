#ifndef BUILD_GRID_H
#define BUILD_GRID_H


#include "SDL_rotozoom.h"
#include "../tools/tools.h"
#include "../list/liste_chainee.h"
#include "../tools/pixel_operations.h"

void concatimage (SDL_Surface *image_surface, SDL_Surface* img2, int x, int y, int coeffx, int coeffy);

void rebuildgrid(SDL_Surface *image_surface, coordonate point_depart, int ligne);

Bool shouldrotate (SDL_Surface* image_surface);
double angularRotation(SDL_Surface *image_surface);
#endif