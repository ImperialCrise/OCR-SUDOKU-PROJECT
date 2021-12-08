#include "grid_detection.h"
#include "../tools/tools.h"
#include "../list/liste_chainee.h"
#include "../tools/pixel_operations.h"

#include <err.h>
#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include<math.h> 

#define THRESHOLDVALUE 125
#define COLORFILL 0

Bool is_black(Uint8 color){
	if(color < THRESHOLDVALUE)
		return true; 
	return false;
}


Uint8 colorpixel(SDL_Surface* image_surface, int w, int h, int i, int j){
	// on recupere la valeur de la couleur du pixel i,j
	if (i>-1 && i<w && j>-1 && j<h){
		Uint32 pixel = get_pixel(image_surface, i, j);
		Uint8 r, g ,b;
		SDL_GetRGB(pixel, image_surface->format, &r, &g, &b);

		return r;
	}
	return 0;
}

void floodfillimagedfs(SDL_Surface *image_surface, int h, int w, int i, int j, Uint8 new_color){

	if(i < 0 || i > w-1 || j < 0 || j > h-1) return;
	
	//printf("pass 1 : (%d, %d) \n", i, j);
	Uint8 actual_color = colorpixel(image_surface, w, h, i, j);

	//printf("pass color : %d \n",colorpixel(image_surface, i, j));
	if (is_black(actual_color) == true) return;	

	//Edit pixel color value
	Uint32 pixelEdit = SDL_MapRGB(image_surface->format, new_color, new_color, new_color);
	//pixel = pixelEdit;
    put_pixel(image_surface, i, j, pixelEdit);

    floodfillimagedfs(image_surface, h, w, i+1, j, new_color);
    //floodfillimagedfs(image_surface, h, w, i, j+1, new_color);//
	floodfillimagedfs(image_surface, h, w, i, j-1, new_color);
	floodfillimagedfs(image_surface, h, w, i-1, j, new_color);//
}

//#define LENLINE 25
Bool foundlineH(SDL_Surface* image_surface, int width, int height, int i, int j, Uint8 new_color){
	int LENLINE = height/30;
	
	int tmp = j;
	while( j < tmp+LENLINE && colorpixel(image_surface, width, height, i, j) != new_color)
	{
		//printf("pass fdl : (%d, %d) \n", i, j);
		j++;
	}
	if (j == tmp + LENLINE) return true;

	//recherche negative
	j = tmp;
	while( j > tmp-LENLINE && colorpixel(image_surface, width, height, i, j) != new_color)
	{
		//printf("pass fdl : (%d, %d) \n", i, j);
		j--;
	}
	if (j == tmp - LENLINE) return true;

	return false;
}

Bool foundlineW(SDL_Surface* image_surface, int width, int height, int i, int j, Uint8 new_color){
	int LENLINE = height/30;

	int tmp = i;
	while( i < tmp+LENLINE && colorpixel(image_surface, width, height, i, j) != new_color)
	{
		//printf("pass fdl : (%d, %d) \n", i, j);
		i++;
	}
	if (i == tmp + LENLINE) return true;

	//recherche negative
	i = tmp;
	while( j > tmp-LENLINE && colorpixel(image_surface, width, height, i, j) != new_color)
	{
		//printf("pass fdl : (%d, %d) \n", i, j);
		i--;
	}
	if (i == tmp - LENLINE) return true;

	return false;
}

void getcoordtopleft(SDL_Surface* image_surface, int width, int height, Uint8 new_color, int res[]){
	int found = -1;

	for (int j = 0; j < height && found == -1; j++)
	{
		for (int i = 0; i < width && found == -1; i++)
		{
			if(colorpixel(image_surface, width, height, i, j) != new_color) {
				res[0] = i;
				res[1] = j;
				found = 0;
				printf("haut gauche %d , %d \n", i, j);
				break;
			}
		}
	}
}

void getcoordtopright(SDL_Surface* image_surface, int width, int height, Uint8 new_color, int res[]){
	int found = -1;

	for (int j = 0; j < height && found == -1; j++)
	{
		for (int i = width; i > 0 && found == -1; i--)
		{
			if(colorpixel(image_surface, width, height, i, j) != new_color) {
				res[0] = i;
				res[1] = j;
				found = 0;
				printf("haut droit %d , %d \n", i, j);
				break;
			}
		}
	}
}

void getcoordbottomleft(SDL_Surface* image_surface, int width, int height, Uint8 new_color, int res[]){
	int found = -1;

	for (int j = height-1 && found == -1; j > 0; j--)
	{
		for (int i = 0; i < width && found == -1; i++)
		{
			if(colorpixel(image_surface, width, height, i, j) != new_color) {
				res[0] = i;
				res[1] = j;
				found = 0;
				printf("bas gauche %d , %d \n", i, j);
				break;
			}
		}
	}
}

void getcoordbottomright(SDL_Surface* image_surface, int width, int height, Uint8 new_color, int res[]){
	int found = -1;

	for (int j = height && found == -1; j > 0; j--)
	{
		for (int i = width && found == -1; i > 0; i--)
		{
			if(colorpixel(image_surface, width, height, i, j) != new_color) {
				res[0] = i;
				res[1] = j;
				found = 0;
				printf("bas droit %d , %d \n", i, j);
				break;
			}
		}
	}
}

int nbwhiteneighbor(SDL_Surface* image_surface, int width, int height, int i, int j, Uint8 new_color){
	int res = 0;
	if(colorpixel(image_surface, width, height, i-1, j-1) != new_color) res ++;
	if(colorpixel(image_surface, width, height, i, j-1) != new_color) res ++;
	if(colorpixel(image_surface, width, height, i+1, j-1) != new_color) res ++;
	if(colorpixel(image_surface, width, height, i-1, j) != new_color) res ++;
	if(colorpixel(image_surface, width, height, i+1, j) != new_color) res ++;
	if(colorpixel(image_surface, width, height, i-1, j+1) != new_color) res ++;
	if(colorpixel(image_surface, width, height, i, j+1) != new_color) res ++;
	if(colorpixel(image_surface, width, height, i+1, j+1) != new_color) res ++;

	return res;
}
/*
float deg2radian(float degree){
	return degree * ( M_PI / 180.0 );
}

float[] arange(float begin, float end){
	float res[end-begin];
	for (float i = begin, int a = 0; i < end; res[a] = i, i++);
	return res;
}

float[] linspace(float start, float stop, int num){
	float res[num] = {start};
	float step = (abs(stop)-abs(start))/(float)num-1;

	for(float i = begin, int a = 0; i < end; res[a] = i+step, i++);
	
	return res;
}


void houghtransform(SDL_Surface* image_surface, int width, int height){
	float thetas = deg2radian(arange(-90.0, 90.0));
	diaglen = ceil(sqrt(width *width + height *height))
	rhos = 
}
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
 
#include <cairo.h>
 
#ifndef M_PI
#define M_PI 3.1415927
#endif
 
#define GR(X,Y) (d[(*s)*(Y)+bpp*(X)+((2)%bpp)])
#define GG(X,Y) (d[(*s)*(Y)+bpp*(X)+((1)%bpp)])
#define GB(X,Y) (d[(*s)*(Y)+bpp*(X)+((0)%bpp)])
#define SR(X,Y) (ht[4*tw*((Y)%th)+4*((X)%tw)+2])
#define SG(X,Y) (ht[4*tw*((Y)%th)+4*((X)%tw)+1])
#define SB(X,Y) (ht[4*tw*((Y)%th)+4*((X)%tw)+0])
#define RAD(A)  (M_PI*((double)(A))/180.0)
uint8_t *houghtransform(uint8_t *d, int *w, int *h, int *s, int bpp)
{
  int rho, theta, y, x, W = *w, H = *h;
  int th = sqrt(W*W + H*H)/2.0;
  int tw = 360;
  uint8_t *ht = malloc(th*tw*4);
  memset(ht, 0, 4*th*tw); // black bg
 
  for(rho = 0; rho < th; rho++)
  {
    for(theta = 0; theta < tw/*720*/; theta++)
    {
      double C = cos(RAD(theta));
      double S = sin(RAD(theta));
      uint32_t totalred = 0;
      uint32_t totalgreen = 0;
      uint32_t totalblue = 0;
      uint32_t totalpix = 0;
      if ( theta < 45 || (theta > 135 && theta < 225) || theta > 315) {
	for(y = 0; y < H; y++) {
	  double dx = W/2.0 + (rho - (H/2.0-y)*S)/C;
	  if ( dx < 0 || dx >= W ) continue;
	  x = floor(dx+.5);
	  if (x == W) continue;
	  totalpix++;
	  totalred += GR(x, y);
	  totalgreen += GG(x, y);
	  totalblue += GB(x, y);
	}
      } else {
	for(x = 0; x < W; x++) {
	  double dy = H/2.0 - (rho - (x - W/2.0)*C)/S;
	  if ( dy < 0 || dy >= H ) continue;
	  y = floor(dy+.5);
	  if (y == H) continue;
	  totalpix++;
	  totalred += GR(x, y);
	  totalgreen += GG(x, y);
	  totalblue += GB(x, y);	  
	}
      }
      if ( totalpix > 0 ) {
	double dp = totalpix;
	SR(theta, rho) = (int)(totalred/dp)   &0xff;
	SG(theta, rho) = (int)(totalgreen/dp) &0xff;
	SB(theta, rho) = (int)(totalblue/dp)  &0xff;
      }
    }
  }
 
  *h = th;   // sqrt(W*W+H*H)/2
  *w = tw;   // 360
  *s = 4*tw;
  return ht;
}
 
int main(int argc, char **argv)
{
  cairo_surface_t *inputimg = NULL;
  cairo_surface_t *houghimg = NULL;
 
  uint8_t *houghdata = NULL, *inputdata = NULL;
  int w, h, s, bpp;
 
  if ( argc < 3 ) return EXIT_FAILURE;
 
  inputimg = cairo_image_surface_create_from_png(argv[1]);
 
  w = cairo_image_surface_get_width(inputimg);
  h = cairo_image_surface_get_height(inputimg);
  s = cairo_image_surface_get_stride(inputimg);  
  bpp = cairo_image_surface_get_format(inputimg);
  switch(bpp)
  {
  case CAIRO_FORMAT_ARGB32: bpp = 4; break;
  case CAIRO_FORMAT_RGB24:  bpp = 3; break;
  case CAIRO_FORMAT_A8:     bpp = 1; break;
  default:
    fprintf(stderr, "unsupported\n");
    goto destroy;
  }
 
  inputdata = cairo_image_surface_get_data(inputimg);
  houghdata = houghtransform(inputdata, &w, &h, &s, bpp);
 
  printf("w=%d, h=%d\n", w, h);
  houghimg = cairo_image_surface_create_for_data(houghdata,
						 CAIRO_FORMAT_RGB24,
						 w, h, s);
  cairo_surface_write_to_png(houghimg, argv[2]);
 
destroy:
  if (inputimg != NULL) cairo_surface_destroy(inputimg);
  if (houghimg != NULL) cairo_surface_destroy(houghimg);
 
  return EXIT_SUCCESS;
}