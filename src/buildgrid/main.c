#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include "../tools/tools.h"

#include "SDL/SDL.h"
#include "SDL/SDL_image.h"

#include "buildgrid.h"
#include "SDL_rotozoom.h"




int main()
{
	SDL_Surface* image_surface;
	

	init_sdl();

	//char path[] = "../tests/grid_image_05.jpeg--Sauvola.bmp";
	char path[] = "../tests/image1.bmp";	
	char pathsauv[] = "../tests/rotatedgrid.bmp";
	//char pathsauv2[] = "../tests/imperfectiongrid.bmp";


	image_surface = load_image(path);

	//SDL_SaveBMP(image_surface, pathsauv2);

	//coordonate point_depart = {420, 169};
	//rebuildgrid(image_surface, point_depart, 130);

	if(shouldrotate(image_surface) == true)
	{
		double angle_rot = angularRotation(image_surface);

	printf("angle de rot : %f\n", angle_rot);
	//printf("%f\n", (90 - angle_rot));
	
	//On tourne de 90 - angularrotation
	// De ce fait, la grille sera tourjour bien oriente

	image_surface = rotozoomSurface(image_surface, angle_rot, 1, 0);
	}

	SDL_SaveBMP(image_surface, pathsauv);

	SDL_FreeSurface(image_surface);

	return 0;
}





/*
//=========================Reduction de l'image===================================

	int width = image_surface -> w;
	int height = image_surface -> h;
	int coeff_reduction_x = width/28;
	int coeff_reduction_y = height/28;


	printf("%d, %d \n\n\n", coeff_reduction_x, coeff_reduction_x);
	image_surface = shrinkSurface(image_surface, coeff_reduction_x, coeff_reduction_y);

	SDL_SaveBMP(image_surface, path2);




//========================Concatenation de 2 images =============================
	concatimage (emptygrid, image_surface, 0, 0, coeff_reduction_x, coeff_reduction_y );

	SDL_SaveBMP(emptygrid, path4);

	SDL_FreeSurface(image_surface);
	SDL_FreeSurface(emptygrid);

*/