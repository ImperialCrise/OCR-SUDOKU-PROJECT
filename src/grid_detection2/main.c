#include <err.h>
#include "SDL/SDL.h"
#include "SDL/SDL_image.h"

#include <stdio.h>
#include <stdlib.h>
#include "grid_detection.h"
#include "../tools/tools.h"
#include "../tools/pixel_operations.h"

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

void wait_for_keypressed()
{
	SDL_Event event;

	// Wait for a key to be down.
	do
	{
		SDL_PollEvent(&event);
	} while(event.type != SDL_KEYDOWN);

	// Wait for a key to be up.
	do
	{
		SDL_PollEvent(&event);
	} while(event.type != SDL_KEYUP);
}

SDL_Surface* display_image(SDL_Surface *img)
{
	SDL_Surface *screen;

	// Set the window to the same size as the image
	screen = SDL_SetVideoMode(img->w, img->h, 0, SDL_SWSURFACE|SDL_ANYFORMAT);
	if (screen == NULL)
	{
		// error management
		errx(1, "Couldn't set %dx%d video mode: %s\n",
				img->w, img->h, SDL_GetError());
	}

	// Blit onto the screen surface
	if(SDL_BlitSurface(img, NULL, screen, NULL) < 0)
		warnx("BlitSurface error: %s\n", SDL_GetError());

	// Update the screen
	SDL_UpdateRect(screen, 0, 0, img->w, img->h);

	// return the screen for further uses
	return screen;
}



int main()
{
	SDL_Surface* image_surface;

	init_sdl();

	char path1[] = 		     "../tests/image_02.jpeg--Sauvola.bmp";
	char path2[] = "../tests/floodfill_image_02.jpeg--Sauvola.bmp";
	char path3[] =      "../tests/grid_image_02.jpeg--Sauvola.bmp";
	char path[100] = "";
	SDL_Surface* grille = NULL;
	SDL_Rect position;
	Uint32 colormax = 0;

	coordonate coord_11 = {-1, -1};
	coordonate coord_12 = {-1, -1};
	coordonate coord_21 = {-1, -1};
	coordonate coord_22 = {-1, -1};

	image_surface = load_image(path1);

	int width = image_surface -> w;
	int height = image_surface -> h;

	SDL_Surface* justgrid = SDL_CreateRGBSurface(0, width,  height, 32, 0, 0, 0, 0);
	Uint32 white_color = SDL_MapRGB(image_surface->format, 255, 255, 255);

	color(image_surface, width, height, &colormax, &coord_11, &coord_12, &coord_21, &coord_22, &white_color);

    //save de l'image
    returngrid(image_surface, justgrid, width, height, &colormax, &white_color);

	SDL_SaveBMP(image_surface, path2);
	SDL_SaveBMP(justgrid, path3);

	int ligne = (coord_21.y - coord_11.y) / 9;
	position.w = ligne;
	position.h = ligne;
	position.y =  coord_11.y;

	//=============================== DECOUPAGE DE LA GRILLE  ===============================
	for (int i = 0; i < 9; i++)
	{
		position.x =  coord_11.x ;
		for (int j = 0; j < 9; j++)
		{
			grille = SDL_CreateRGBSurface(0, ligne, ligne, 32, 0, 0, 0, 0);

			if (SDL_BlitSurface(image_surface, &position, grille, NULL) != 0)
			{        
				fprintf(stderr, "%s\n", "Erreur lors de l'allocation dynamique memoire. \n" );
				exit(EXIT_FAILURE);
			}

			//A faire
			colorimperfection(grille, ligne, ligne, white_color);
			snprintf(path, 100, "../tests/grilleimage/grille%d%d.jpeg--Sauvola.bmp", j, i);

			//resize to 28 x 28
			//grille = rotozoomSurface(grille, 0, 2, 1); 

			SDL_SaveBMP(grille, path);
			SDL_FreeSurface(grille);

			position.x += ligne;
		}
		position.y += ligne;
	}

	SDL_FreeSurface(image_surface);
	SDL_FreeSurface(justgrid);

	return 0;

}



