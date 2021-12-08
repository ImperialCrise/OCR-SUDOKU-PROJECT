#include "loader.h"



#define DFT_W 20
#define DFT_MIDW 10
#define DEFAULT_K 0.20
#define DEFAULT_R 128

// =================== GREYSCALE ========================

void toGreyScale(SDL_Surface *image){

	int h = image->h;
	int w = image->w;

	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			Uint32 pixel = get_pixel(image, x, y);

			Uint8 r, g, b;
			SDL_GetRGB(pixel, image->format, &r, &g, &b);

			r = (r+g+b) / 3;

			pixel = SDL_MapRGB(image->format, r, r, r);

			put_pixel(image, x, y, pixel);
		}
	}
}



// =================== GAUSS BLUR ========================

void toGaussBlurFilter(SDL_Surface *image){

	int h = image->h;
	int w = image->w;

	int midh = h / 2;
	int midw = w / 2;

	double sigma = 1.0;
	double r, s = 2.0 * sigma * sigma;
	double sum = 0.0;

	for (int y = -midh; y < midh; y++)
	{
		for (int x = -midw; x < midw; x++)
		{
			r = sqrt(x * x + y * y);
			Uint32 pixel = ((-(r * r) / s)) / (M_PI * s);
			
			put_pixel(image, x + midw, y + midh, pixel);

			sum += pixel;
		}
	}

	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			Uint32 pixel = get_pixel(image, x, y);

			pixel /= sum;

			put_pixel(image, x, y, pixel);
		}
	}
}

// =================== Sauvola ========================
unsigned long int GetColorPixel(SDL_Surface *image, int x, int y){
	Uint32 pixel = get_pixel(image, x, y);
	Uint8 r, g, b;		
	SDL_GetRGB(pixel, image->format, &r, &g, &b);

	return (unsigned long int) r;
}

unsigned long int GetColorPixelSquared(SDL_Surface *image, int x, int y){
	
	unsigned long int r = GetColorPixel(image, x, y);
	return r * r;

}

unsigned long int** SauvolaImageIntegralSquared(SDL_Surface *image){

	size_t h = image->h;
	size_t w = image->w;

	unsigned long int** matrix = malloc(w*sizeof(unsigned long int*));

	for (size_t i=0; i<w; i++)
		matrix[i] = calloc(h, sizeof(unsigned long int));
 
	matrix[0][0] = GetColorPixelSquared(image, 0, 0);
	
	//printf( "%lu\n", matrix[0][0]);

	for (size_t y = 1; y < h; y++)
		matrix[0][y] = matrix[0][y-1] + GetColorPixelSquared(image, 0, y);

	for (size_t x = 1; x < w; x++){
		matrix[x][0] = matrix[x-1][0] + GetColorPixelSquared(image, x, 0);
		//printf( "%lu\n", matrix[x][0]);
	}

	for (size_t y = 1; y < h; y++)
		for (size_t x = 1; x < w; x++)
			matrix[x][y] = matrix[x-1][y] + matrix[x][y-1] - matrix[x-1][y-1] + GetColorPixelSquared(image, x, y);

	return matrix;
}

unsigned long int** SauvolaImageIntegral(SDL_Surface *image){

	size_t h = image->h;
	size_t w = image->w;

	unsigned long int** matrix = malloc(w*sizeof(unsigned long int*));

	for (size_t i=0; i<w; i++)
		matrix[i] = calloc(h, sizeof(unsigned long int));

	matrix[0][0] = GetColorPixel(image, 0, 0);
	
	for (size_t y = 1; y < h; y++)
		matrix[0][y] = matrix[0][y-1] + GetColorPixel(image, 0, y);

	for (size_t x = 1; x < w; x++)
		matrix[x][0] = matrix[x-1][0] + GetColorPixel(image, x, 0);

	for (size_t y = 1; y < h; y++)
		for (size_t x = 1; x < w; x++)
			matrix[x][y] = matrix[x-1][y] + matrix[x][y-1] - matrix[x-1][y-1] + GetColorPixel(image, x, y);

	return matrix;
}

float SauvolaVariance(SDL_Surface *image, unsigned long int m, int h, int w, int X, int Y){

	unsigned long int sum = 0;

	int xmin = X - DFT_MIDW < 0 ? 0 : X - DFT_MIDW;
	int xmax = X + DFT_MIDW >= w ? w : X + DFT_MIDW;

	int ymin = Y - DFT_MIDW < 0 ? 0 : Y - DFT_MIDW;
	int ymax = Y + DFT_MIDW >= h ? h : Y + DFT_MIDW;

	for (int x = xmin; x < xmax; x++)
	{
		for (int y = ymin; y < ymax; y++)
		{
			unsigned long int a = GetColorPixel(image, x, y) - m;
			sum += a * a;
		}
	}

	return (float) sqrt(sum / ((xmax - xmin) * (ymax - ymin)));
}

unsigned long int g(unsigned long int** mat, int h, int w, int y, int x){

	if (x > w)
		x = w;
	if (y > h)
		y = h;

	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;

	return mat[x][y];
}

// =================== Median Blur ========================

void toMedianBlur(SDL_Surface *image, unsigned long int** mat){

	int h = image->h-1;
	int w = image->w-1;
	int r;

	for (int y = 1; y < h; y++)
	{
		for (int x = 1; x < w; x++)
		{
			float m = (float)
				(g(mat, h, w, y + 1, x + 1)
				-g(mat, h, w, y + 1, x - 1)
				-g(mat, h, w, y - 1, x + 1)
				+g(mat, h, w, y - 1, x - 1)
				)/5;

			Uint8 rg, gr, br;
			SDL_GetRGB(get_pixel(image, x, y), image->format, &rg, &gr, &br);

			r = m < rg ? 255 : 0;

			put_pixel(image, x, y, SDL_MapRGB(image->format, r, r, r));
		}
	}


}

void SauvolaThresholding(SDL_Surface *image){
	unsigned long int** mat = SauvolaImageIntegral(image);
	unsigned long int** matSquared = SauvolaImageIntegralSquared(image);

	int h = image->h-1;
	int w = image->w-1;

	for (int y = 1; y < h; y++)
	{
		for (int x = 1; x < w; x++)
		{
			Uint8 r, gr, br;

			SDL_GetRGB(get_pixel(image, x, y), image->format, &r, &gr, &br);

			float xmin = (float)x - DFT_MIDW < 0 ? 0 : (float)x - DFT_MIDW;
			float xmax = (float)x + DFT_MIDW >= (float)w ? (float)w : (float)x + DFT_MIDW;
			float ymin = (float)y - DFT_MIDW < 0 ? 0 : (float)y - DFT_MIDW;
			float ymax = (float)y + DFT_MIDW >= (float)h ? (float)h : (float)y + DFT_MIDW;

			float m = (float)
				(g(mat, h, w, y + DFT_MIDW, x + DFT_MIDW)
				-g(mat, h, w, y + DFT_MIDW, x - DFT_MIDW)
				-g(mat, h, w, y - DFT_MIDW, x + DFT_MIDW)
				+g(mat, h, w, y - DFT_MIDW, x - DFT_MIDW)
				)/((xmax - xmin) * (ymax - ymin));

			float variance =(float)
				(g(matSquared, h, w, y + DFT_MIDW, x + DFT_MIDW)
				-g(matSquared, h, w, y + DFT_MIDW, x - DFT_MIDW)
				-g(matSquared, h, w, y - DFT_MIDW, x + DFT_MIDW)
				+g(matSquared, h, w, y - DFT_MIDW, x - DFT_MIDW)
				)/((xmax - xmin) * (ymax - ymin));
 
			variance = sqrt((variance-m*m));

			//printf( "var => %f\n", SauvolaVariance(image, m, h, w, x, y));
			//printf( "var => %f\n\n",variance);
			//float var = sqrt(((m)- (m * m))/(DFT_W*DFT_W));

			unsigned long int calc = m * (1 + DEFAULT_K * (float) (variance/DEFAULT_R - 1));

			if (r <= calc)
			{
				r = 0;
			}else{
				r = 255;
			}


			put_pixel(image, x, y, SDL_MapRGB(image->format, r, r, r));
		}
	}

	toMedianBlur(image, mat);

	for (int i=0; i<w+1; i++)
		free(mat[i]); 
	free(mat);

	for (int i=0; i<w+1; i++)
		free(matSquared[i]); 
	free(matSquared);

	

 }

