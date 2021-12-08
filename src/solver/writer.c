#include <stdio.h>
#include "writer.h"


void writer(char argv[],int grid[9][9]){
    FILE * fPtr;
    strcat(argv, ".result");
    fPtr = fopen(argv, "w");
    for (size_t y = 0; y < 9; y++)
    {
        for (size_t x = 0; x < 9; x++)
        {
            
            fprintf(fPtr,"%d",grid[y][x]);
            if ((x + 1) %3 == 0 && x != 0)
            {
                fprintf(fPtr," ");
            }
        }
        if (y !=8)
        {
            fprintf(fPtr,"\n");
        }
        
        
        if ((y == 2 || y == 5)  )
        {
            fprintf(fPtr,"\n");
        }
        
        
    }
}