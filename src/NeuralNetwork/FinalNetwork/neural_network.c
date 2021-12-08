#include "neural_network.h"
#include "neural_network_tool.h"



struct Neural_Network* InitializeNet()
{
	struct Neural_Network *net = 
	malloc(sizeof(struct Neural_Network));
	net->nbInput = 28*28;
	net->nbHidden = 500;
	net->nbOutput = 10;
	net->ErrorRate = 0.0;
	net->eta = 0.01;
	net->alpha = 0.1;
  

	for (int i = 0; i < net->nbInput; i++)
	{
		for (int j = 0; j< net->nbHidden; j++)
		{
			net->dWeightIH[i][j] = 0.0;
			net->WeightIH[i][j] = Random();
		}
		
	}
	

	for (int i = 0; i < net->nbHidden; i++)
	{
		for (int j = 0; j < net->nbOutput; j++)
		{
			net->WeightHO[i][j] = Random();
			net->dWeightHO[i][j] = 0.0;
		}
		net->BiasH[i] = Random();
		
	}

	for (int i = 0; i < net->nbOutput; i++)
	{
		net->BiasO[i] = Random();
		net->dOutputO[i] = 0.0;
	}
	return net;
};

void ForwardPass(struct Neural_Network *net){
	double sum;
	double weight;
	double output;
	double bias;

	for (int i = 0; i < net->nbHidden; i++)
	{
		sum = 0.0;
		for (int j = 0; j < net->nbInput;j++){
			weight = net->WeightIH[j][i];
			output = net->ImageBin[j];

			sum += weight * output;
		}
		bias = net->BiasH[i];
		net->OutputH[i] = Sigmoid(sum + bias);
	}
	for (int i = 0; i < net->nbOutput; i++)
	{
		sum = 0.0;
		for (int j = 0; j < net->nbHidden;j++){
			weight = net->WeightHO[j][i];
			output = net->OutputH[j];

			sum += weight * output;
		}
		bias = net->BiasO[i];
		net->OutputO[i] = Sigmoid(sum + bias);
	}

}

void BackwardPass(struct Neural_Network *net){

	double output;
	double derivative;
	double error;
	double goal;

	//Cost of output neurons
	for(int i = 0; i < net->nbOutput;i++){
		output = net->OutputO[i];
		derivative = Derivative_Sigmoid(output);
		goal = net->Goal[i];
		error = (goal - output) * derivative;

		net->dOutputO[i] = error;

	}
	//Cost of hidden neurons
	double sum;
	double weight;
	double delta;
	for (int i = 0; i < net->nbHidden; i++)
	{
		sum = 0.0;
		for (int j = 0; i < net->nbOutput; j++)
		{
			weight = net->WeightHO[i][j];
			delta = net->dOutputO[j];
			sum += weight * delta;
		}
		output = net->OutputH[i];
		derivative = Derivative_Sigmoid(output);
		net->dHidden[i] = sum * derivative;
	}    
}

void UpdateWeights(struct Neural_Network *net)
{
  double eta = net->eta;
  double alpha = net->alpha;
  double error;
  double output;
  double dWeight;
  for (int i = 0; i < net->nbHidden; i++)
  {
	for(int j = 0; j < net->nbInput; j++)
	{
	  output = net->ImageBin[j];
	  error = net -> dHidden[i];
	  dWeight = net -> dWeightIH[j][i];

	  net->WeightIH[j][i] += eta * error * output +
								alpha * dWeight;
	  net->dWeightIH[j][i] = eta * error * output;
	}
  }

  
  for (int i = 0; i < net->nbOutput; i++)
  {
	for (int j = 0; j < net->nbHidden; j++)
	{
	  output = net->OutputH[j];
	  error = net->dOutputO[i];
	  dWeight = net->dWeightHO[j][i];

	  net->WeightHO[j][i] += eta * error * output +
								alpha * dWeight;
	  net->dWeightHO[j][i] = eta * error * output;
	}
  }
}

void UpdateBiases(struct Neural_Network *net)
{
  double eta = net->eta;
  double delta;
  double dBias;
  for (int i = 0; i < net->nbHidden; i++)
  {
	delta = net->dHidden[i];
	dBias = eta * delta;

	net->BiasH[i] += dBias;
  }
  for (int i= 0;  i< net->nbOutput; i++)
  {
	delta = net->dOutputO[i];
	dBias = eta * delta;

	net->BiasO[i] += dBias;
  }
}

void SquaredErrorRate(struct Neural_Network *net)
{
  for (int i = 0; i < net->nbOutput; i++)
  {
	net->ErrorRate += 0.5 * (net->Goal[i] - net->OutputO[i])* (net->Goal[i] - net->OutputO[i]);
  }
}

void SaveData(struct Neural_Network *net)
{
  FILE* weightIH = fopen("../../../datas/neural/weightIH.weight", "w");
  for(int i = 0; i < net->nbInput; i++)
  {
	for(int j = 0; j < net->nbHidden; j++)
	{
		
	  fprintf(weightIH, "%f\n", net->WeightIH[i][j]);
	}
  }
  fclose(weightIH);
  printf("Test1\n");

  FILE* weightHO = fopen("../../../datas/neural/weightHO.weight", "w");
  for(int i = 0; i < net->nbHidden; i++)
  {
	for(int j = 0; j < net->nbOutput; j++)
	{
		fprintf(weightHO, "%f\n", net->WeightHO[i][j]);
	}
  }
  fclose(weightHO);
  printf("Test1\n");

  FILE* biasH = fopen("../../../datas/neural/biasH.bias", "w");
  for(int i = 0; i < net->nbHidden; i++)
  {
	fprintf(biasH, "%f\n", net->BiasH[i]);
  }
  fclose(biasH);
  printf("Test1\n");

  FILE* biasO = fopen("../../../datas/neural/biasO.bias", "w");
  for (int i = 0; i < net->nbOutput; i++)
  {
	  fprintf(biasO, "%f\n", net->BiasO[i]);
  }
  fclose(biasO);
  printf("Test1\n");
}

void replace(char* line)
{
	for (int i = 0; line[i] != '\0'; ++i)
		if (line[i] == '.')
			line[i] = ',';
}

void replace2(char* line)
{
	for (int i = 0; line[i] != '\0'; ++i)
		if (line[i] == ',')
			line[i] = '.';
}


struct Neural_Network* Load_Data_from_files(int a){

  struct Neural_Network *net = NULL;
	net = malloc(sizeof(struct Neural_Network));
	net -> nbInput = 28*28;
	net -> nbHidden = 500;
	net -> nbOutput = 10;
	int sizeMax = 15;

	char *eptr;
	char* line = NULL; 
	size_t len = 0; 
	ssize_t read; 

	(void) read;
	(void) eptr;
	(void) sizeMax;
	FILE* weightIH = fopen("./datas/neural/weightIH.weight", "r+");
	for(int i = 0; i < net->nbInput; i++) 
	  { 
		for(int j = 0; j < net->nbHidden; j++) 
	 { 
	 	read = getline(&line, &len, weightIH);
	   strtok(line, "\n"); 
	   if (a == 1)
	   	replace(line);
	   else
	   	replace2(line);
	   
	   net -> WeightIH[i][j] = atof(line); 
	 } 
   } 
   fclose(weightIH); 

 FILE* weightHO = fopen("./datas/neural/weightHO.weight", "r+");
  for(int i = 0; i < net->nbHidden; i++)
  {
	for(int j = 0; j < net->nbOutput; j++)
	{
		read = getline(&line, &len, weightHO);

	   strtok(line, "\n"); 
	   if (a == 1)
	   	replace(line);
	   else
	   	replace2(line);
	   net -> WeightHO[i][j] = atof(line);
	}
  }
  fclose(weightHO);

  FILE* biasH = fopen("./datas/neural/biasH.bias", "r+");
  for(int i = 0; i < net->nbHidden; i++)
  {
	read = getline(&line, &len, biasH);
	strtok(line, "\n"); 
	   if (a == 1)
	   	replace(line);
	   else
	   	replace2(line);
	net->BiasH[i] = atof(line);
  }
  fclose(biasH);

  FILE* biasO = fopen("./datas/neural/biasO.bias", "r+");
  for (int i = 0; i < net->nbOutput; i++)
  {
  	read = getline(&line, &len, biasO);

	strtok(line, "\n"); 
	   if (a == 1)
	   	replace(line);
	   else
	   	replace2(line);
	net->BiasO[i]= atof(line);
  }
  fclose(biasO);



  return net;



}

void From_image_to_table(struct Data_test *test,int p, SDL_Surface* image_surface){
	init_sdl2();

	int height = image_surface->h;
	int width = image_surface->w;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			Uint32 color = get_pixel2(image_surface,j,i);
			Uint8 r,g,b;
			SDL_GetRGB(color,image_surface->format,&r,&g,&b);
			if (r == 0)
			{
				test->input[i*width +j] = 1.0;
			}
			else{
				test->input[i*width +j] = 0.0;
			}
			
		}
	}
	for (int i = 0; i < 10; i++)
	{
		test->goal[i] = 0.0;
	}
	test->goal[p] = 1.0;
}

void InitTraining(struct Neural_Network *net, struct Data_test *test){
	for (int i = 0; i < net->nbInput; i++)
	{
		net->ImageBin[i] = test->input[i];
	}
	for (int i = 0; i < net->nbOutput; i++)
	{
		net->Goal[i] = test->goal[i];
	}
}

void PrintInput(struct Neural_Network *net)
{
  for (int i = 0; i < 28; i++)
  {
	for (int j = 0; j < 28; j++)
	{
	  printf("%d",(int)net->ImageBin[i*28 +j]);
	}
	printf("\n");
  }
  
};

void PrintOutput(struct Neural_Network *net){
  for (int i = 0; i < net->nbOutput; i++)
  {
	printf("Output[%d] => %f \n",i,net->OutputO[i]);
  }
  

};

int RetrieveResult(struct Neural_Network *net){
	double max = 0.0;
	int result = 0;

	int isTrue = 0;
	for (int i = 0; i < net->nbInput; ++i){
		if (net->ImageBin[i] == 1){
			isTrue = 1;
			break;
		}
	}
		
	if (isTrue == 0)
		return 0;

	for (int i = 0; i < net->nbOutput; i++)
	{
		if (max < net->OutputO[i])
		{
			result = i;
			max = net->OutputO[i];
		}
	}
	return result;
}

void writer(char argv[],int grid[9][9]){
	FILE * fPtr;
	fPtr = fopen(argv, "w");
	for (size_t y = 0; y < 9; y++)
	{
		for (size_t x = 0; x < 9; x++)
		{
			if (grid[x][y] == 0)
			{
				fprintf(fPtr," ");
			}
			else{
			fprintf(fPtr,"%d",grid[x][y]);
			}
		}
		if (y !=8)
		{
			fprintf(fPtr,"\n");
		}   
	}

	fclose(fPtr); 
}


void NeuralNet(int grid[9][9], SDL_Surface* liste[9][9], int a)
{
	struct Neural_Network *net = InitializeNet();
	net = Load_Data_from_files(a);

	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			char pathstr[100] = "";
			snprintf(pathstr, 100, "./datas/tmp/grille/grille%d%d.bmp", i, j);
			struct Data_test test;
			From_image_to_table(&test, i+1%9, liste[i][j]);
			InitTraining(net,&test);
			ForwardPass(net);
			grid[i][j] = RetrieveResult(net);
		}
	}

	char *path = "./datas/tmp/grid_made";
	writer(path,grid);
	free(net);
}


