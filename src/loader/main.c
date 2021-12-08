#include "loader.h"

int main(int argc, char * argv[])
{

	(void) argc;
	(void) argv;

	int max = 9;
	char path[max][50];

	for (int i = 0; i < max; i++)
	{
		char pathstr[50] = "";
		snprintf(pathstr, 50, "../../shrinked_database/new_img%02d-00005.png", i+2);
        strcpy(path[i], pathstr);
	}

	for (int i = 0; i < max; ++i)
	{
		printf("%s\n", path[i]);
	}



	return 0;
}