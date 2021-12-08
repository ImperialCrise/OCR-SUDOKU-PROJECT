#include "buildgrid.h"
#include "../tools/tools.h"
#include "../tools/pixel_operations.h"

#include <err.h>
#include "SDL/SDL.h"
#include "SDL/SDL_image.h"



// On se place aux coord x, y et pour chaque pixel dans imgtoconcat, on copie sa couleur dans image_surface
void concatimage (SDL_Surface *image_surface, SDL_Surface* img2, int x, int y, int coeffx, int coeffy){
    //on commence par remettre l'image a niveau. X28
    img2 = zoomSurface(img2, (double) coeffx, (double) coeffy, 0);
    int w1 = image_surface -> w;
    int h1 = image_surface -> h;

    int w2 = img2 -> w;
    int h2 = img2 -> h;
    printf("%d, %d \n", w2, h2);

    Uint32 actual_color = 0;

    if(x > w1-1 || y > h1-1 || x+w2 > w1-1 || y+h2 > h1-1) return; // si on sort du cadre


    for (int i = x, a = 0; i < x+w2; i++, a++)
    {
        for (int j = y, b = 0; j < y+h2; j++, b++)
        {
            actual_color = get_pixel(img2, a, b);
            put_pixel(image_surface, i, j, actual_color);
        }
    }
}

void rebuildgrid(SDL_Surface *image_surface, coordonate point_depart, int ligne){
    
    SDL_Surface *casegrille;

    init_sdl();
    char pathcase[100];

    
    int x = point_depart.x;
    int y = point_depart.y;

    int completeline = 9*ligne;

    for (int i = x, a = 0; i < x+completeline ; i+=ligne, a++)
    {
        for (int j = y, b = 0; j < y+completeline ; j+=ligne, b++)
        {
            snprintf(pathcase, 100, "../tests/grilleimage/grille%d%d.jpeg--Sauvola.bmp", a, b);
            casegrille = load_image(pathcase);

            concatimage(image_surface, casegrille, i, j, 1, 1);
        }
    }
}

int powpow(int i, int j, int Px, int Py){
    int pa = (i - Px);
    int pb = (j - Py);

    return pa*pa + pb*pb;
}

void getcoordtoprightleft(SDL_Surface* image_surface, Uint32 color, coordonate* coord_top, coordonate* coord_right, coordonate* coord_left){
    int w = image_surface -> w; 
    int h = image_surface -> h;

    int ctop = 0, cright = 0, cleft = 0;
    // Parcour successif des lignes de la matrices depuis 0,0

    for (int i = 0; ctop == 0 &&  i < h-1; i++)
    {
        for (int j = 0; ctop == 0 && j < w-1; j++)
        {
            if (get_pixel(image_surface, j, i) == color)
            {     

                *coord_top = (coordonate){j, i};
                ctop = 1;
            }
        }
    }

    // Parcour successif des colonnes de la matrices depuis w,0
    for (int i = w-1; cright == 0 && i > 0; i--)
    {

        for (int j = 0; cright == 0 && j < h; j++)
        {
            if (get_pixel(image_surface, i, j) == color)
            {           

                *coord_right = (coordonate){i, j};
                cright = 1;           
            }
        }
    }

    // Parcour successif des colonnes de la matrices depuis 0,0
    for (int i = 0;  cleft == 0 && i < w-1; i++)
    {

        for (int j = 0; cleft == 0 && j < h; j++)
        {
            if (get_pixel(image_surface, i, j) == color)
            {  

                *coord_left = (coordonate){i, j};
                cleft = 1;           
            }
        }
    }

}

int power2(int x){
    return x * x;
}

double angularRotation(SDL_Surface *image_surface){

    coordonate a = {0,0}, b = a, c = a;

    getcoordtoprightleft(image_surface, 0, &a, &b, &c);

    int xa = a.x, ya = a.y, xb = b.x, yb = b.y, xo = xa, yo = yb, xc = c.x, yc = c.y;

    int ab = sqrt( power2(xb - xa) + power2(yb -ya));
    int oa = sqrt( power2(xo - xa) + power2(yo -ya));

    int co = sqrt( power2(xc - xo) + power2(yc -yo));
    int ob = sqrt( power2(xo - xb) + power2(yo -yb));

    double cos_oab = (double)oa / (double)ab;
 
    if (co < ob ) return 90 - (acos(cos_oab)*180 / 3.14); // rotation droite
    
    //printf("test de valeur %d %d\n", xa, ya );
    return - (acos(cos_oab)*180 / 3.14);

}

Bool foundlineW(SDL_Surface* image_surface, int i, int j, int w, int h, int longueur, Uint32 blackcolor){
    int tmp = i;

    if(i+longueur > w-1 || j+longueur > h-1 || i < 0 || j < 0) return false; // si on sort du cadre
    
    while( i < tmp+longueur && get_pixel(image_surface, i, j) == blackcolor)
    {
        //printf("pass fdl : (%d, %d) \n", i, j);
        i++;
    }
    if (i == tmp + longueur) return true;

    return false;
}

Bool shouldrotate (SDL_Surface* image_surface){
    int w = image_surface -> w; 
    int h = image_surface -> h;

    int longueur = 120; //just for test

    //Uint32 white_color = SDL_MapRGB(image_surface->format, 255, 255, 255);


    // Parcour successif des lignes de la matrices depuis 0,0 et on supp l
    for (int i = 0; i < h; i++)
    {
        for (int j = 0; j < w; j++)
        {
            if (get_pixel(image_surface, j, i) == 0 && foundlineW(image_surface, j, i, w, h, longueur, 0) == true )
            {
                return false;   
            }
        }
    }

    return true;
}
