#include "test_save_Xor.h"


struct Xor_Neural_Network
{
    int nbInput;
    int nbHidden;
    int nbOutput;

    //Arrays
    double InputValue[8];
    double Goal[4];
    //Weight Arrays IH -> Input to Hidden / HO Hidden to Output
    double WeightIH[4];
    double WeightHO[2];
    //Bias Hidden Array
    double BiasH[2];
    //Bias output
    double BiasO;
    //Output Hidden
    double OutputH[2];
    //Output of output
    double OutputO;
    //Array of delta bias for hidden
    double dBiasH[2];
    //delta bias for output
    double dBiasO;
    //delta weight
    double dWeightIH[4];
    double dWeightHO[4];
    //delta of output
    double dOutputO;
    //delta of hidden
    double dHidden[2];
    
    double ErrorRate;
    //learning rate for everything except hidden
    double eta;
    //learning rate for hidden
    double alpha;
};

static double Random()
{
    return (double)rand()/(double)RAND_MAX;
}

static double sigmoid(double x)
{
  return(1.0/(1.0+exp(-x)));
}

struct Xor_Neural_Network InitalizeNetwork()
{
  struct Xor_Neural_Network net;
  net.nbInput = 2;
  net.nbHidden = 2;
  net.BiasO = Random();
  net.OutputO = 0;
  net.dBiasO = 0.0;
  net.dOutputO = 0.0;  
  net.ErrorRate = 0.0;
  net.eta = 0.5;
  net.alpha = 0.9;
  return(net);
}
void InitalizeValue(struct Xor_Neural_Network *net)
{
  
  (*net).InputValue[0] = 0;
  (*net).InputValue[1] = 0;
  (*net).InputValue[2] = 0;
  (*net).InputValue[3] = 1;
  (*net).InputValue[4] = 1;
  (*net).InputValue[5] = 0;
  (*net).InputValue[6] = 1;
  (*net).InputValue[7] = 1;
  
  (*net).Goal[0] = 0;
  (*net).Goal[1] = 1;
  (*net).Goal[2] = 1;
  (*net).Goal[3] = 0;


  for (int i = 0;i< (*net).nbInput; i++)
  {
    for (int h = 0; h < (*net).nbHidden; ++h)
    {
      (*net).WeightIH[(h + i * (*net).nbHidden)] = Random();
      (*net).dWeightIH[(h + i * (*net).nbHidden)]  = 0.0;
    }
  }
  
  for (int h = 0; h < (*net).nbHidden; ++h)
  {
    (*net).WeightHO[h] = Random();
    (*net).dWeightHO[h] = 0.0;
    (*net).BiasH[h] = Random();
    (*net).dBiasH[h]  = 0.0;
  }
  (*net).dBiasO= 0.0;
}

void ForwardPass(struct Xor_Neural_Network *net, int p,int epoch)
{
  for (int h = 0; h <  (*net).nbHidden; h++)
  {
    double SumIH = 0.0;
    for (int i = 0; i < (*net).nbInput; i++)
    {
      SumIH += (*net).WeightIH[(h+i*(*net).nbHidden)] * 
                (*net).InputValue[(i+p*(*net).nbInput)];
    }
    (*net).OutputH[h] = sigmoid(SumIH +  (*net).BiasH[h]);
  }
  double SumHO = 0;
  for (int h = 0; h < (*net).nbHidden; ++h)
  {
    SumHO += (*net).WeightHO[h] * (*net).OutputH[h];
  }
  (*net).OutputO = sigmoid(SumHO + (*net).BiasO);

  //Squared error function
  (*net).ErrorRate += 0.5 * ((*net).Goal[p] - (*net).OutputO) * 
                      ((*net).Goal[p] - (*net).OutputO);
  if (epoch % 100 == 0)
  {
    switch (p)
    {
    case 1:
        printf("Case 01 | Input 1 : %f | Input 2 : %f | Output: %f \n"
                    ,(*net).InputValue[p*2]
                    ,(*net).InputValue [p*2 +1], (*net).OutputO);
        break;
    case 2:
        printf("Case 10 | Input 1 : %f | Input 2 : %f | Output: %f \n"
                    ,(*net).InputValue[p*2]
                    ,(*net).InputValue [p*2 +1], (*net).OutputO);
        break;
    case 3:
        printf("Case 11 | Input 1 : %f | Input 2 : %f | Output: %f \n"
                    ,(*net).InputValue[p*2]
                    ,(*net).InputValue [p*2 +1], (*net).OutputO);
        break;
    
    default:
        printf("Case 00 | Input 1 : %f | Input 2 : %f | Output: %f \n"
                    ,(*net).InputValue[p*2]
                    ,(*net).InputValue [p*2 +1], (*net).OutputO);
        break;
    }
  }
}

void BackwardPass(struct Xor_Neural_Network *net,int p)
{
  (*net).dOutputO = ((*net).Goal[p] - (*net).OutputO) *
                      (*net).OutputO * (1.0 - (*net).OutputO);
  for (int h = 0; h < (*net).nbHidden; h++)
  {
    double dSumOutput = (*net).WeightHO[h] * (*net).dOutputO ;
    (*net).dHidden[h] = dSumOutput * (*net).OutputH[h] * 
                            (1.0 - (*net).OutputH[h]);
  }
  //Update weights & bias between Input and Hidden layers
  for (int h = 0; h < (*net).nbHidden; ++h)
  {
    //Update BiasH
    (*net).dBiasH[h] = (*net).eta *  (*net).dHidden[h];
    (*net).BiasH[h] += (*net).dBiasH[h] ;
    for (int i = 0; i < (*net).nbInput; ++i)
    {
      //Update WeightIH
      (*net).dWeightIH[(h+i*(*net).nbHidden)] = (*net).eta * 
                                                (*net).InputValue
                                                [(i+p*(*net).nbInput)]  
                                                *
                                                (*net).dHidden[h] + 
                                                (*net).alpha * 
                                                (*net).dWeightIH
                                                [(h+i*(*net).nbHidden)];

      (*net).WeightIH[(h+i*(*net).nbHidden)] += (*net).dWeightIH
                                                [(h+i*(*net).nbHidden)];
    }
  }
    //Update weights & bias between Hidden and Ouput layers
  //Update BiasO
  (*net).dBiasO = (*net).eta * (*net).dOutputO;
  (*net).BiasO  += (*net).dBiasO ;
  for (int h = 0; h < (*net).nbHidden; ++h)
  {
    //Update WeightsHO
    (*net).dWeightHO[h] = (*net).eta * (*net).OutputH[h] *
                              (*net).dOutputO + (*net).alpha *
                              (*net).dWeightHO[h];
    *((*net).WeightHO + h) += (*net).dWeightHO[h] ;
  }
}

void SaveData(struct Xor_Neural_Network *net)
{
  FILE* weightIH = fopen("weightIH.weight", "w");
  for(int i = 0; i < (*net).nbInput; i++)
  {
    for(int j = 0; j < (*net).nbHidden; j++)
    {
        double d = (*net).WeightIH[i +j];
      fprintf(weightIH, "%f\n", d);
    }
  }
  fclose(weightIH);

  FILE* weightHO = fopen("weightHO.weight", "w");
  for(int i = 0; i < (*net).nbHidden; i++)
  {
    for(int j = 0; j < (*net).nbOutput; j++)
    {
        fprintf(weightHO, "%f\n", (*net).WeightHO[i + j]);
    }
  }
  fclose(weightHO);

  FILE* biasH = fopen("biasH.bias", "w");
  for(int i = 0; i < (*net).nbHidden; i++)
  {
    fprintf(biasH, "%f\n", (*net).BiasH[i]);
  }
  fclose(biasH);

  FILE* biasO = fopen("biasO.bias", "w");
  fprintf(biasO, "%f\n", (*net).BiasO);
  
  fclose(biasO);
}

void XOR()
{
  srand(time(NULL));
  
  int NbPattern = 4;
  int NbEpoch = 10000;

  struct Xor_Neural_Network net_1 = InitalizeNetwork();
  struct Xor_Neural_Network *net = &net_1;

  InitalizeValue(net);
  for (int epoch = 0; epoch <= NbEpoch; epoch++)
  {
    (*net).ErrorRate = 0.0;
    for (int p = 0; p < NbPattern; ++p)
    {
      ForwardPass(net,p,epoch);
      BackwardPass(net,p);
    }
    //== PRINT ==//
    if (epoch % 100 == 0)
    {
      printf("Epoch %-5d == ErrorRate = %f\n", epoch, (*net).ErrorRate);
    }
    
  }
  printf("Saving ...");

    SaveData(net);
}


