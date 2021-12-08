#include "grid_detection.h"
#include "../tools/tools.h"
#include "../list/liste_chainee.h"
#include "../tools/pixel_operations.h"

#include <err.h>
#include "SDL/SDL.h"
#include "SDL/SDL_image.h"

#define THRESHOLDVALUE 125
#define MINPIXELLINE 10

/**
 * @ return : true if black and false else 
 * */
Bool is_black(int color){
	if(color < THRESHOLDVALUE)
		return true; 
	return false;
}

/**
 * param boolW : a boolean equal to true if we read a line and false if we read a column
 * @ return : an unidimensional array containing the coordonnate of the beginning to the ending of the column
 * */

/*
coordonate *createtable(int tailletab, int i, int j, Bool boolW){
	
	static coordonate table[tailletab] = {0};

	if (boolW == true) 
	{
		for (int a = 0; a < tailletab; a++)
		{
			coordonate coord = {i, j};
			table[a] = coord;
			i++;
		}
	}
	else{
		for (int a = 0; a < tailletab; a++)
		{
			coordonate coord = {i, j};
			table[a] = coord;
			j++;
		}
	}
}
*/

/**
 * @**matrice : image binaire matricielle
 * @ return : a list containing the coordonnate of the beginning to the ending of the line
 * */
List foundlineW(int **matrice_image, int width, int *x, int *y)
{
	List res = new_list();

	while(*x < width-1 && matrice_image[*x][*y] == 1) // tant que on a une ligne de 1
	{
		coordonate coord = {*x, *y};
		res = push_back_list(res, coord);
		*x = *x+1;
	}

	if (list_length(res) > MINPIXELLINE)
		return res;

	return NULL;
}


/**
 *  * @**matrice : image binaire matricielle
 * @ return : a list containing the coordonnate of the beginning to the ending of the column\
 *             or nothing if the list is not enough long
 * */
List foundlineH(int **matrice_image, int height, int *x, int *y)
{
	List res = new_list();

	while(*y < height-1 && matrice_image[*x][*y] == 1) // tant que on a une ligne de 1
	{
		res = push_back_list(res, (coordonate){*x, *y});
		*y = *y+1;
	}
	if (list_length(res) > MINPIXELLINE)
		return res;

	return NULL;
}

/**
 * WARNING il est important de separer les boucles for car on passe les index par adresse
 * @**matrice : image binaire matricielle 
 * @ return : a list of verticals an horizontals lines arrays coordonnate
 * */
List listoftable(int **matrice_image, int width, int height)
{
	int *pi;
	int *pj;
	
	List listtable = new_list();
	

	for (int i = 0, j = 0; i < width && j < height; ++i)
	{
	
		for (j = 0; j < height; j++)
		{
			pi = &i;
			pj = &j;
			listtable = add_list(listtable, foundlineW(matrice_image, width, pi, pj));
		}

		
	
	}

	for (int i = 0; i < width; ++i)
	{

		for (int j = 0; j < height; j++)
		{    
			pi = &i;
			pj = &j;      
			listtable = add_list(listtable, foundlineH(matrice_image, height, pi, pj));
		}
	
	}

	return listtable;
}

/**
 * On parcours la liste et on retire les valeurs presentent moins de 2 fois
 * @ return : a list of verticals an horizontals lines arrays coordonnate
 * */
List crosscoordonates(List listtable)
{
	ListElement *temp = listtable;

	while (temp!=NULL)
	{
		if (value_repetition(listtable, temp -> value) < 2)
		{
			listtable = pop_list_value(listtable, temp -> value);
		}
		
		temp = temp->next;
	}

	return listtable;  
}


int *coordonatetableau(List listtable)
{
	int i, imax, j, jmax;

	ListElement *temp = listtable;
	i = imax = j = jmax = 0;

	while(temp!=NULL){
		if (temp->value.x < i)
			i = temp->value.x;

		if (temp->value.x > imax)
			imax = temp->value.x;
		
		if (temp->value.y < j)
			j = temp->value.y;

		if (temp->value.y > jmax)
			jmax = temp->value.y;

		temp = temp->next;
	}

	int* res = malloc(4 * sizeof(int));

	res[0] = i;
	res[1] = imax;
	res[2] = j;
	res[3] = jmax;

	printf("j = %d, jmax = %d\n", j,jmax);

	return res;
}






