#ifndef GRILLE_DETECTION_H
#define GRILLE_DETECTION_H

#include "../tools/tools.h"
#include "../list/liste_chainee.h"
#include "../tools/pixel_operations.h"

Bool is_black(int color);

//coordonate *createtable(int tailletab, int i, int j, Bool boolW);
List foundlineW(int **matrice_image, int width, int *x, int *y);
List foundlineH(int **matrice_image, int height, int *x, int *y);
List listoftable(int **matrice_image, int width, int height);

List crosscoordonates(List listtable);
int *coordonatetableau(List listtable);


#endif