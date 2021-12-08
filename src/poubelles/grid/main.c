#include <err.h>
#include "SDL/SDL.h"
#include "SDL/SDL_image.h"

#include <stdio.h>
#include <stdlib.h>
#include "grid_detection.h"
#include "../list/liste_chainee.h"
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
	//SDL_Surface* screen_surface;

	init_sdl();

	image_surface = load_image("../tests/image_02.jpeg--Sauvola.bmp");
	
	//screen_surface = display_image(image_surface);

	//wait_for_keypressed();

	int width = image_surface -> w;
	int height = image_surface -> h;


	// on cree une matrice contenant la valeur binaire de chaque pixel de l'image
	
	//int matrice_image[width][height];
	int **matrice_image = malloc(width * sizeof(int*));
	for (int i = 0; i < width; i++)
		matrice_image[i] = calloc(height, sizeof( int));


	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			Uint32 pixel = get_pixel(image_surface, i, j);

			Uint8 r, g ,b;
			SDL_GetRGB(pixel, image_surface->format, &r, &g, &b);

			if (is_black(r) == false)  //pixel blanc
				matrice_image[i][j] = 0;
			else
				matrice_image[i][j] = 1;
		}
	}

	List listtable = listoftable(matrice_image, width, height);
	//printf("test5\n");
	listtable = crosscoordonates(listtable);
	
	print_list(listtable);

	int *res = coordonatetableau(listtable);

	SDL_Surface *grille;
	SDL_Rect position;

	int widhtimage = res[1]-res[0];
	int heightimage = res[3]-res[2];

	position.w = widhtimage;
	position.h = heightimage;

	position.x = res[0];
	position.y = res[2];

	printf("w=%d, h=%d\n", widhtimage, heightimage);
	//printf("w2=%d, h2=%d\n", position.x-widhtimage, position.y - heightimage);

	
	grille = SDL_CreateRGBSurface(0, widhtimage, heightimage, 32, 0, 0, 0, 0);

	if (SDL_BlitSurface(image_surface, &position, grille, NULL) != 0)
	{        
		fprintf(stderr, "%s\n", "Erreur lors de l'allocation dynamique memoire. \n" );
		exit(EXIT_FAILURE);
	}
	
	printf("test7\n");
	SDL_SaveBMP(grille, "../tests/grille.bmp");
	printf("test8\n");
	//screen_surface = display_image(grille);

	//update_surface(screen_surface, grille);
	
	//wait_for_keypressed();

//SDL_FreeSurface(screen_surface);
	SDL_FreeSurface(image_surface);
	SDL_FreeSurface(grille);
	//SDL_Quit();

	printf("Finished\n");


	return 0;
}