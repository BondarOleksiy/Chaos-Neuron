#include<stdio.h>
#include"network.h"

#define INPUTS_NUM  2
#define CHAOS_DEPTH 5

#define PATTERNS_NUM 4
#define EPOCHS       100

void make_xls(double *y, int points, char *file_name)
{
	FILE *fp;
	char buf[100];
	char name[100];
	strcpy(name,file_name);
	strcat(name,".xls");
	
	if ((fp=fopen(name, "wb"))==NULL)
	{
		printf("Cannot create file.\n");
	}
	
	 for(int i=0;i<points;i++)
	{
		//gcvt(y[i], 20, buf);
		sprintf(buf, "%f", y[i]);
		
		fwrite(buf, 7, 1, fp);
		fwrite("\n", sizeof(char), 1, fp);
	}
	
	fclose (fp);

    printf("xls made");
}

void main()
{
	srand(time(NULL));
	
	cell *chaon = init_cell(INPUTS_NUM, CHAOS_DEPTH);
	
	double train_input[PATTERNS_NUM][INPUTS_NUM];
	double train_output[PATTERNS_NUM];
	
	//train patterns for XOR
	train_input[0][0] = 0;// pat0
	train_input[0][1] = 0;
	train_output[0]   = 0;
	
	train_input[1][0] = 1;// pat1
	train_input[1][1] = 0;
	train_output[1]   = 1;
	
	train_input[2][0] = 0;// pat2
	train_input[2][1] = 1;
	train_output[2]   = 1;
	
	train_input[3][0] = 1;// pat3
	train_input[3][1] = 1;
	train_output[3]   = 0;
	//train patterns for XOR
	
	double delta_out;// dE/Axon_out
	double error_array[EPOCHS];
	double error_temp[PATTERNS_NUM];
	double max_error;
	
	for(int i=0;i<EPOCHS;i++)
	{
		for(int j=0;j<PATTERNS_NUM;j++)
		{
			delta_out = run_cell(chaon,train_input[j]) - train_output[j];
			train_cell(chaon,train_input[j],delta_out);
			
			if(delta_out<0)error_temp[j] = -delta_out;
			else error_temp[j] = delta_out;
		}
		
		max_error = error_temp[0];
		for(int j=1;j<PATTERNS_NUM;j++)
		{
			if(max_error<error_temp[j])max_error=error_temp[j];
		}
		
		error_array[i] = max_error;
	}
	double result;
	for(int i=0;i<PATTERNS_NUM;i++)
	{
		result = run_cell(chaon,train_input[i]);
		printf("Pat_%d: in_0 %f\t in_1 %f\t out %f\n", i, train_input[i][0], train_input[i][1], result);
	}
	
	make_xls(error_array, EPOCHS, "error_10");	
}
