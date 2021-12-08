#include <stdio.h>
#include "reader.h"
#include <stdlib.h>

void reader(char argv[], int grid[9][9]){
    FILE *inputFile;
    inputFile = fopen(argv,"r");
    char ch;
    int x = 0;
    int y = 0;
    while(x < 9){
        ch = fgetc(inputFile);
        if (ch != '\n' && ch != 32)
        {
            if (ch == '.')
            {
                grid[x][y] = 0;
            }
            else{
               grid[x][y] = ch - 48; 
            }
            y = y + 1;
            if (y > 8)
            {
                x = x + 1;
                y = 0;
            }
        }
    }
    fclose(inputFile);
}