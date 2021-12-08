#include "neural_network.h"
#include "neural_network_tool.h"

int main(){
  srand(time(NULL));  
  /* Training 
  struct Neural_Network *net = InitializeNet();
  
 
  int max = 40;
  int k = 0;
    char path[max][100];
    for (int j = 0; j < 4; j++)
    {
      
    

    for (int i = 0; i < 10; i++)
    {
        char pathstr[100] = "";
        snprintf(pathstr, 100, "../../../datas/Training_Data/img_0%02d-0000%d.bmp", i+1,j+1);
        strcpy(path[k], pathstr);
        k = k+1;
    }}


  struct Data_test* list = calloc(40,sizeof(struct Data_test));
  
  for (int i = 0; i < 40; i++)
  {
    struct Data_test test;
      From_image_to_table(&test,i%10,path[i]);
      list[i] = test;
  }
  int epoch = 0;
  net->ErrorRate = 0.03; 

  while(net->ErrorRate > 0.02){ 
    net->ErrorRate = 0.0; 
    for (int i = 0; i < max; ++i)
  {
    InitTraining(net,&list[i]);
    ForwardPass(net);
    SquaredErrorRate(net);
    BackwardPass(net);
    UpdateWeights(net);
    UpdateBiases(net);
    if (epoch %100 == 0){
      printf("Epoch => %d \n",epoch);
      PrintOutput(net);
      printf("Error Rate => %f\n",net->ErrorRate);
  }
  
    }
    epoch += 1;
  }
  printf("Epoch => %d \n",epoch);
  PrintOutput(net);
  printf("Error Rate => %f\n",net->ErrorRate);
  printf("Training Finished\n");
  SaveData(net);*/
  
 char* path1 = "../../../datas/neural/weightIH.weight";
  char* path2 = "../../../datas/neural/weightIH.weight";
  char* path3 = "../../../datas/neural/weightIH.weight";
  char* path4 = "../../../datas/neural/weightIH.weight";

  //char* pathprint = "../../datas/tmp/grille/grille%d%d.bmp";

  char* pathgrid = "../../../datas/tmp/grid_made";

 
  NeuralNet(path1,path2,path3,path4, pathgrid);


    return 0;
}