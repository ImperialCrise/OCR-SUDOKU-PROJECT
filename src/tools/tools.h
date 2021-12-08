#ifndef TOOLS_H
#define TOOLS_H

#include <err.h>
#include "SDL/SDL.h"
#include "SDL/SDL_image.h"

typedef struct coordonate
{
    int x, y;
} coordonate;


typedef enum
{
    false,
    true    
}Bool;


// proto
Bool is_coordonate_equal(coordonate *coord1, coordonate *coord2);


void init_sdl();

SDL_Surface* load_image(char *path);

#endif
