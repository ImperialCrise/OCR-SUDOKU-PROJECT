#include "neural_network_tool.h"

double Random(){

  return (((double)rand()) / ((double)RAND_MAX / 2) - 1)/2;
}

double Sigmoid(double x)
{
  return(1.0 / (1.0 + exp(-x)));
}

double Derivative_Sigmoid(double x)
{
  return x * (1.0 - x);
}

void init_sdl2()
{
  // Init only the video part.
  // If it fails, die with an error message.
  if(SDL_Init(SDL_INIT_VIDEO) == -1)
    errx(1,"Could not initialize SDL: %s.\n", SDL_GetError());
}
Uint8* pixel_ref2(SDL_Surface *surf, unsigned x, unsigned y)
{
    int bpp = surf->format->BytesPerPixel;
    return (Uint8*)surf->pixels + y * surf->pitch + x * bpp;
}

SDL_Surface* load_image2(char *path)
{
  SDL_Surface *img;

  // Load an image using SDL_image with format detection.
  // If it fails, die with an error message.
  img = IMG_Load(path);
  if (!img)
    errx(3, "can't load %s: %s", path, IMG_GetError());

  return img;
}

Uint32 get_pixel2(SDL_Surface *surface, unsigned x, unsigned y)
{
    Uint8 *p = pixel_ref2(surface, x, y);

    switch (surface->format->BytesPerPixel)
    {
        case 1:
            return *p;

        case 2:
            return *(Uint16 *)p;

        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                return p[0] << 16 | p[1] << 8 | p[2];
            else
                return p[0] | p[1] << 8 | p[2] << 16;

        case 4:
            return *(Uint32 *)p;
    }

    return 0;
}
