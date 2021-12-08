#ifndef NEURAL_NETWORK_H
#define NEURAL_NETWORK_H
#include "neural_network_tool.h"

#define HIDDENSIZE 500
 
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <SDL/SDL.h>

struct Neural_Network
{
    int nbInput;
    int nbHidden;
    int nbOutput;

    double ImageBin[28*28];
    double Goal[10];

    double WeightIH[28*28][HIDDENSIZE];
    double WeightHO[HIDDENSIZE][10];

    double BiasH[HIDDENSIZE];

    double BiasO[10];

    double OutputH[HIDDENSIZE];

    double OutputO[10];

    double dWeightIH[28*28][HIDDENSIZE];
    double dWeightHO[HIDDENSIZE][10];

    double dOutputO[10];

    double dHidden[HIDDENSIZE];

    double ErrorRate;
    double eta;
    double alpha;
};

struct Neural_Network* InitializeNet();

void From_image_to_table(struct Data_test *test,int p,SDL_Surface* liste);

void InitTraining(struct Neural_Network *net, struct Data_test *test);

void PrintInput(struct Neural_Network *net);

void PrintOutput(struct Neural_Network *net);

Uint32 get_pixel2(SDL_Surface *surface, unsigned x, unsigned y);

void SquaredErrorRate(struct Neural_Network *net);

void SaveData(struct Neural_Network *net);

struct Neural_Network* Load_Data_from_files();

void NeuralNet();

void ForwardPass(struct Neural_Network *net);

void BackwardPass(struct Neural_Network *net);

void UpdateWeights(struct Neural_Network *net);

void UpdateBiases(struct Neural_Network *net);

int RetrieveResult(struct Neural_Network *net);

void writer(char argv[],int grid[9][9]);


# endif