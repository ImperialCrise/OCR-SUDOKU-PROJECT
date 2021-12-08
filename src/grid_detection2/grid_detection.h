#ifndef GRILLE_DETECTION_H
#define GRILLE_DETECTION_H

#include "../tools/tools.h"
#include "../list/liste_chainee.h"
#include "../tools/pixel_operations.h"

void cutting(SDL_Surface* image_surface, SDL_Surface* cutted_image, SDL_Rect* position, Uint32* white_color, SDL_Surface* liste[9][9]);
void floodfill_black_plus1(SDL_Surface *image_surface, int w, int h, int i, int j, Uint32* white_color);
void floodfill_black_plus2(SDL_Surface *image_surface, int w, int h, int i, int j, Uint32* white_color);
void floodfill_black_minus1(SDL_Surface *image_surface, int w, int h, int i, int j, Uint32* white_color);
void floodfill_black_minus2(SDL_Surface *image_surface, int w, int h, int i, int j, Uint32* white_color);
void floodfillimagedfs(SDL_Surface *image_surface, int w, int h, int i, int j, Uint32* new_color, Uint32* condition, int* nbrcolored);
void color(SDL_Surface* image_surface, int w, int h, Uint32* colormax, coordonate* coord_11, coordonate* coord_12, coordonate* coord_21, coordonate* coord_22, int add);
void getcoord(SDL_Surface* image_surface, int w, int h, Uint32 colormax, coordonate* coord_11, coordonate* coord_12, coordonate* coord_21, coordonate* coord_22);
void colorimperfection(SDL_Surface* image_surface, int w, int h, Uint32* white_color);
void returngrid(SDL_Surface* image_surface, SDL_Surface* grid, int w, int h, Uint32* color, Uint32* white_color);
void drawSudoku(SDL_Surface* image_surface, int width, int height);
#endif