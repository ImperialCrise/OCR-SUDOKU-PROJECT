#include <stdio.h>
#include <stdlib.h>
#include "tools.h"

Bool is_coordonate_equal(coordonate *coord1, coordonate *coord2)
{
	
	if (coord1 -> x == coord2 -> x && coord1 -> y == coord2 -> y)
		return true;

	return false;
}

void init_sdl()
{
	// Init only the video part.
	// If it fails, die with an error message.
	if(SDL_Init(SDL_INIT_VIDEO) == -1)
		errx(1,"Could not initialize SDL: %s.\n", SDL_GetError());
}

SDL_Surface* load_image(char *path)
{
	SDL_Surface *img;

	// Load an image using SDL_image with format detection.
	// If it fails, die with an error message.
	img = IMG_Load(path);
	if (!img)
		errx(3, "can't load %s: %s", path, IMG_GetError());

	return img;
}