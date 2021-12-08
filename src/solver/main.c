#include <stdio.h>
#include <err.h>
#include "reader.h"
#include "solver.h"
#include "writer.h"

#define SIZE 9

void print(int arr[9][9]);

int main(int argc, char *argv[]){
    if (argc != 2)
    {
      errx(1,"Please input the right argument");  
    }
    int grid[SIZE][SIZE] = {0};
    
    reader(argv[1], grid);
    solveSudoku(grid,0,0);
    writer(argv[1],grid);
    
}
/*
//I need that because i'm dumb and can't even make a sudoku solver properly.
void print(int arr[9][9])
{
     for (int i = 0; i < 9; i++)
      {
         for (int j = 0; j < 9; j++)
            printf("%d ",arr[i][j]);
         printf("\n");
       }
}
*/