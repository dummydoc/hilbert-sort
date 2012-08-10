#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
	FILE* file;
	FILE* outfile;
	double x,y;
	
	if(argc == 3)
	{
		file = fopen(argv[1],"r");
		outfile = fopen(argv[2],"w");
		fflush(stdout);
		
		while(fscanf(file, "\t%lf\t%lf",&x, &y) != EOF)
		{
			fwrite(&x, sizeof(double), 1, outfile);
			fwrite(&y, sizeof(double), 1, outfile);	
		}

		fclose(file);
	
	
	}
	return 0;
}
