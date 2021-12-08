#include <err.h>
#include <math.h> 

#include "grid_detection.h"
#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL_rotozoom.h"
#include "../tools/tools.h"
#include "../list/liste_chainee.h"
#include "../tools/pixel_operations.h"


void floodfill_black_plus1(SDL_Surface *image_surface, int w, int h, int i, int j, Uint32* white_color)
{
	if(i < 0 || i > w || j < 0 || j > h || get_pixel(image_surface, i, j) != 0) 
		return;

	put_pixel(image_surface, i, j, *white_color);

	floodfill_black_plus1(image_surface, w, h, i+1, j, white_color);
	floodfill_black_plus1(image_surface, w, h, i, j+1, white_color);
}

void floodfill_black_plus2(SDL_Surface *image_surface, int w, int h, int i, int j, Uint32* white_color)
{
	if(i < 0 || i > w || j < 0 || j > h || get_pixel(image_surface, i, j) != 0) 
		return;

	put_pixel(image_surface, i, j, *white_color);

	floodfill_black_plus2(image_surface, w, h, i-1, j, white_color);
	floodfill_black_plus2(image_surface, w, h, i, j+1, white_color);
}

void floodfill_black_minus1(SDL_Surface *image_surface, int w, int h, int i, int j, Uint32* white_color)
{
	if(i < 0 || i > w || j < 0 || j > h || get_pixel(image_surface, i, j) != 0) 
		return;

	put_pixel(image_surface, i, j, *white_color);

	floodfill_black_minus1(image_surface, w, h, i-1, j, white_color);
	floodfill_black_minus1(image_surface, w, h, i, j-1, white_color);
}

void floodfill_black_minus2(SDL_Surface *image_surface, int w, int h, int i, int j, Uint32* white_color)
{
	if(i < 0 || i > w || j < 0 || j > h || get_pixel(image_surface, i, j) != 0) 
		return;

	put_pixel(image_surface, i, j, *white_color);

	floodfill_black_minus2(image_surface, w, h, i+1, j, white_color);
	floodfill_black_minus2(image_surface, w, h, i, j-1, white_color);
}


void floodfillimagedfs(SDL_Surface *image_surface, int w, int h, int i, int j, Uint32* new_color, Uint32* condition, int* nbrcolored)
{
	if(i < 0 || i > w || j < 0 || j > h || get_pixel(image_surface, i, j) != *condition) 
		return;

	*nbrcolored += 1;
	put_pixel(image_surface, i, j, *new_color);

	floodfillimagedfs(image_surface, w, h, i+1, j, new_color, condition, nbrcolored);
	floodfillimagedfs(image_surface, w, h, i-1, j, new_color, condition, nbrcolored);
	floodfillimagedfs(image_surface, w, h, i, j+1, new_color, condition, nbrcolored);
	floodfillimagedfs(image_surface, w, h, i, j-1, new_color, condition, nbrcolored);	
}

SDL_Surface* centerAndZoom(SDL_Surface* image_surface, Uint32* white_color)
{
	int width = image_surface->w;
	int height = image_surface->h;

	int y1 = height;
	int x2 = 0; 
 	int y3 = 0;
	int x4 = width;

	for (int i = 0; i < width; ++i)
	{
		for (int j = 0; j < height; ++j)
		{
			if (get_pixel(image_surface, i, j) == 0)
			{
				if (j <= y1) y1 = j;
				if (j >= y3) y3 = j;

				if (i <= x4) x4 = i;
				if (i >= x2) x2 = i;
			}
		}
	}

	SDL_Rect position;
	position.x = x4;
	position.y = y1;
	position.w = x2 > x4 ? (x2-x4) : (x4-x2);
	position.h = y3 > y1 ? (y3-y1) : (y1-y3) ;

	Bool isNumber = false;
	int size;
	if (position.w > position.h)
	{
		isNumber = false;
		size = position.w;
	}else{
		isNumber = true;
		size = position.h;
	}
	
	SDL_Surface* image = SDL_CreateRGBSurface(0, size, size, 32, 0, 0, 0, 0);

	for (int i = 0; i < size; ++i)
	{
		for (int j = 0; j < size; ++j)
		{
			put_pixel(image, i, j, *white_color);
		}
	}

	SDL_Rect position1;
	if (isNumber)
	{
		position1.x = size/2 - position.w/2;
		position1.y = 0;
	}else{
		position1.x = 0;
		position1.y = size/2 - position.h/2;
	}

	if (isNumber && SDL_BlitSurface(image_surface, &position, image, &position1) != 0)
	{        
		fprintf(stderr, "%s\n", "Erreur lors de l'allocation dynamique memoire. \n" );
		exit(EXIT_FAILURE);
	}

	SDL_FreeSurface(image_surface);

	return image;
}

void colorimperfection(SDL_Surface* image_surface, int w, int h, Uint32* white_color)
{
	Uint32 colormax, paintcolor, condition = 0;
	int max = (int)((float)95*(float)w/(float)100), nbr;

	for (int i = 0; i < w; i++)
	{
		for (int j = 0; j < h; j++)
		{
			nbr = 0;
			paintcolor = get_pixel(image_surface, i, j);

			if (paintcolor != *white_color && paintcolor > condition)
			{
				floodfillimagedfs(image_surface, w-1, h-1, i, j, &condition, &paintcolor, &nbr);
				if (max < nbr)
				{
					max = nbr;
					colormax = condition;
				}

				condition++;
			}
		}
	}

	for (int i = 0; i < w; i++)
		for (int j = 0; j < h; j++)
			put_pixel(image_surface, i, j, get_pixel(image_surface, i, j) == colormax ? 0 : *white_color);


}

void colorimperfectionCutted(SDL_Surface* image_surface, SDL_Surface* cutted_image, SDL_Rect position, int w, int h)
{
	int wt = position.x + w/2;
	int ht = position.y + h/2;

	for (int i = 0; i < w; i++)
		for (int j = 0; j < h; j++)
			if (get_pixel(image_surface, i, j) == 0)
				put_pixel(cutted_image, i + wt, j + ht, 0);	
}


void cutting(SDL_Surface* image_surface, SDL_Surface* cutted_image, SDL_Rect* position, Uint32* white_color, SDL_Surface* liste[9][9])
{
	SDL_Surface* grille = NULL;

	char path[100] = "";
	int ligne = position->w / 9;

	position->w = ligne;
	position->h = ligne;
	position->y = 0;

	for (int i = 0; i < 9; i++)
	{
		position->x = 0;
		for (int j = 0; j < 9; j++)
		{
			grille = SDL_CreateRGBSurface(0, ligne, ligne, 32, 0, 0, 0, 0);

			if (SDL_BlitSurface(image_surface, position, grille, NULL) != 0)
			{        
				fprintf(stderr, "%s\n", "Erreur lors de l'allocation dynamique memoire. \n" );
				exit(EXIT_FAILURE);
			}

			colorimperfection(grille, ligne, ligne, white_color);

			//printf("%d%d\n", j, i);
			snprintf(path, 100, "./datas/tmp/grille/grille%d%d.bmp", j, i);
			
			grille = centerAndZoom(grille, white_color);

			colorimperfectionCutted(grille, cutted_image, *position, grille->w, grille->w);

			double coef = (double)28/(double)grille->w;
			grille = rotozoomSurfaceXY(grille, 0, coef, coef, 1);

			liste[i][j] = grille;
			SDL_SaveBMP(grille, path);

			position->x += ligne;
		}
		position->y += ligne;
	}
}

void drawSudoku(SDL_Surface* image_surface, int width, int height)
{
	// coloriage de la grille en rouge
	SDL_Rect r1 = {0, 0, width, 10};
	SDL_Rect r4 = {0, 0, 10, height};
	SDL_Rect r2 = {width-10, 0, 10, height};
	SDL_Rect r3 = {0, height-10, width, 10};

	Uint32 pixel = SDL_MapRGB(image_surface->format, 246,177,100);
	int part = width / 9;

	SDL_FillRect(image_surface, &r1, pixel);
	SDL_FillRect(image_surface, &r2, pixel);
	SDL_FillRect(image_surface, &r3, pixel);
	SDL_FillRect(image_surface, &r4, pixel);

	r1.x = part;
	r1.y = 0;
	r1.w = 5;
	r1.h = height;

	for (int j = 0; j < 3; ++j)
	{	
		for (int i = 0; i < 2; ++i)
		{
			SDL_FillRect(image_surface, &r1, pixel);
			r1.x += part;
		}
		r1.w = 10;
		SDL_FillRect(image_surface, &r1, pixel);
		r1.x += part;
		r1.w = 5;
	}

	r1.x = 0;
	r1.y = part;
	r1.w = width;
	r1.h = 5;

	for (int j = 0; j < 3; ++j)
	{	
		for (int i = 0; i < 2; ++i)
		{
			SDL_FillRect(image_surface, &r1, pixel);
			r1.y += part;
		}
		r1.h = 10;
		SDL_FillRect(image_surface, &r1, pixel);
		r1.y += part;
		r1.h = 5;
	}
}



void color(SDL_Surface* image_surface, int w, int h, Uint32* colormax, coordonate* coord_11, coordonate* coord_12, coordonate* coord_21, coordonate* coord_22, int add)
{
	Uint32 paintcolor = 6; // couleur a appliquer sur le pixel actuel
	Uint32 condition = 0;
	int max = -1;
	int nbr = 0;

	for (int i = 0; i < w; i++)
	{
		for (int j = 0; j < h; j++)
		{
			nbr = 0;

			if (get_pixel(image_surface, i, j) == 0)
			{
				floodfillimagedfs(image_surface, w-1, h-1, i, j, &paintcolor, &condition, &nbr);
				if (max < nbr)
				{
					max = nbr;
					*colormax = paintcolor;
				}

				paintcolor += add;
			}
		}
	}

	getcoord(image_surface, w, h, *colormax, coord_11, coord_12, coord_21, coord_22);
}



int powpow2(int i, int j, int Px, int Py){
	int pa = (i - Px);
	int pb = (j - Py);

	return pa*pa + pb*pb;
}

void getcoord(SDL_Surface* image_surface, int w, int h, Uint32 colormax, coordonate* coord_11, coordonate* coord_12, coordonate* coord_21, coordonate* coord_22){
	
	int dist_11 = w*h, dist_12 = dist_11, dist_21 = dist_11,  dist_22 = dist_11;
	int tmp_dist_11, tmp_dist_12, tmp_dist_21, tmp_dist_22;

	for (int i = 0; i < w; i++)
	{
		for (int j = 0; j < h; j++)
		{
			if (get_pixel(image_surface, i, j) == colormax)
			{
				tmp_dist_11 = powpow2(i, j, 0, 0);
				tmp_dist_12 = powpow2(i, j, w, 0);
				tmp_dist_21 = powpow2(i, j, w, h);
				tmp_dist_22 = powpow2(i, j, 0, h);

				if (tmp_dist_11 < dist_11)
				{
					dist_11 = tmp_dist_11;
					*coord_11 = (coordonate){i, j};
				}

				if (tmp_dist_12 < dist_12)
				{
					dist_12 = tmp_dist_12;
					*coord_12 = (coordonate){i, j};
				}

				if (tmp_dist_21 < dist_21)
				{
					dist_21 = tmp_dist_21;
					*coord_21 = (coordonate){i, j};
				}

				if (tmp_dist_22 < dist_22)
				{
					dist_22 = tmp_dist_22;
					*coord_22 = (coordonate){i, j};
				}
			}
		}
	}
}

void returngrid(SDL_Surface* image_surface, SDL_Surface* grid, int w, int h, Uint32* maxcolor, Uint32* white_color){
	for (int i = 0; i < w; i++){
		for (int j = 0; j < h; j++){
			if (get_pixel(image_surface, i, j) == *maxcolor)
			{
				put_pixel(grid, i, j, 0);
				put_pixel(image_surface, i, j, *white_color);
			}else{
				put_pixel(grid, i, j, *white_color);
			}
		}
	}		
}
