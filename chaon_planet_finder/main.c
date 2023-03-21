#include <stdio.h>
#include "database.h"
#include "network.h"

#define PATTERN_SIZE 1024            //points for pattern
#define SPECTRUM_SIZE PATTERN_SIZE/2

#define HIDDEN_NEURONS 5
#define CHAOS_DEPTH    10

#define TRAIN_REPEATS 10000
#define EPOCHS        10

char positive_database_address[]    ={"fits_database/planet_positive"};
char negative_database_address[]    ={"fits_database/planet_negative"};
char negative_ver_database_address[]={"fits_database/planet_negative_ver"};
char positive_ver_database_address[]={"fits_database/planet_positive_ver"};
char candidates_database_address[]  ={"fits_database/candidates"};

void train_network_database(network *net, database *base, int epochs)
{
	for(int i=0;i<epochs;i++)
	{
		for(int j=0;j<base->patterns_num;j++)
		{
			train_network_pattern(net, base->input[j]->spectrum, base->output+j);
		}	
	}
}

double check_database_max_error(network *net, database *base)
{
	double max_error;
	double temp;
	
	temp = base->output[0] - run_network(net, base->input[0]->spectrum);
	if(temp<0)temp *= -1;
	max_error = temp;
	
	for(int i=1;i<base->patterns_num;i++)
	{
		temp = base->output[i] - run_network(net, base->input[i]->spectrum);
		if(temp<0)temp *= -1;
		
		if(max_error<temp)max_error = temp;
	}
	return(max_error);	
}

double check_database_avg_error(network *net, database *base)
{
	double avg_error=0;
	double temp;

	for(int i=0;i<base->patterns_num;i++)
	{
		temp = base->output[i] - run_network(net, base->input[i]->spectrum);
		if(temp<0)temp *= -1;
		
		avg_error += temp;
	}
	avg_error /= base->patterns_num;
	return(avg_error);	
}

int calc_percent(int input, int max)
{
	input *= 100;
	input /= max;
	return(input);
}

void make_database_report(database *base, network *net, char *name)// makes xls file with network estimations on database
{
	FILE *fp;
	char buf[300];
	
	if ((fp=fopen(name, "wb"))==NULL)
	{
		printf("Cannot create file.\n");
	}
	
	for(int i=0;i<base->patterns_num;i++)
	{
		fwrite(base->input[i]->name, strlen(base->input[i]->name), 1, fp);
		fwrite("\t", sizeof(char), 1, fp);
		sprintf(buf, "%f", run_network(net, base->input[i]->spectrum));
		fwrite(buf, 7, 1, fp);
		fwrite("\n", sizeof(char), 1, fp);
	}
	
	fclose (fp);

    printf("report done\n");
}

int main()
{
	printf("ANN init...\n");	
	network *spectro_net = init_network(SPECTRUM_SIZE, HIDDEN_NEURONS, CHAOS_DEPTH);//spectrum network
	printf("Init Sucsess.\n");	
	
	printf("\n");
	
	printf("Extract Positive Database...\n");
	database *positive_base = init_database(positive_database_address,PATTERN_SIZE);//contains pat and spec
	for(int i=0;i<positive_base->patterns_num;i++) positive_base->output[i] = POSITIVE;
	printf("Sucsess.\n");
	printf("Base input patterns: %d\n", positive_base->patterns_num);
	
	printf("\n");
	
	printf("Extract Negative Database...\n");
	database *negative_base = init_database(negative_database_address,PATTERN_SIZE);//contains pat and spec
	for(int i=0;i<negative_base->patterns_num;i++) negative_base->output[i] = NEGATIVE;
	printf("Sucsess.\n");
	printf("Base input patterns: %d\n", negative_base->patterns_num);
	
	printf("\n");
	
	printf("Extract Positive ver Database...\n");
	database *positive_ver_base = init_database(positive_ver_database_address,PATTERN_SIZE);//contains pat and spec
	for(int i=0;i<positive_ver_base->patterns_num;i++) positive_ver_base->output[i] = POSITIVE;
	printf("Sucsess.\n");
	printf("Base input patterns: %d\n", positive_ver_base->patterns_num);
	
	printf("\n");
	
	printf("Extract Negative ver Database...\n");
	database *negative_ver_base = init_database(negative_ver_database_address,PATTERN_SIZE);//contains pat and spec
	for(int i=0;i<negative_ver_base->patterns_num;i++) negative_ver_base->output[i] = NEGATIVE;
	printf("Sucsess.\n");
	printf("Base input patterns: %d\n", negative_ver_base->patterns_num);
	
	printf("\n");
	
	printf("Extract Candidates Database...\n");
	database *candidates_base = init_database(candidates_database_address,PATTERN_SIZE);//contains pat and spec
	for(int i=0;i<candidates_base->patterns_num;i++) candidates_base->output[i] = POSITIVE;
	printf("Sucsess.\n");
	printf("Base input patterns: %d\n", candidates_base->patterns_num);
	
	printf("\n");
	
	double pos_max_err;// max errors
	double neg_max_err;
	double pos_ver_max_err;
	double neg_ver_max_err;
	
	double pos_avr_err;// average errors
	double neg_avr_err;
	double pos_ver_avr_err;
	double neg_ver_avr_err;
	
	for(int i=0;i<TRAIN_REPEATS;i++)
	{
		printf("Processed: %d%%\n", calc_percent(i,TRAIN_REPEATS));
		
		train_network_database(spectro_net, positive_base, EPOCHS);	
		train_network_database(spectro_net, negative_base, EPOCHS);
		
		pos_max_err = check_database_max_error(spectro_net, positive_base);
		pos_avr_err = check_database_avg_error(spectro_net, positive_base);
		printf("pos_max_err: %f\t \t pos_avr_err: %f\n", pos_max_err, pos_avr_err);
		
		neg_max_err = check_database_max_error(spectro_net, negative_base);
		neg_avr_err = check_database_avg_error(spectro_net, negative_base);
		printf("neg_max_err: %f\t \t neg_avr_err: %f\n", neg_max_err, neg_avr_err);
		
		pos_ver_max_err = check_database_max_error(spectro_net, positive_ver_base);
		pos_ver_avr_err = check_database_avg_error(spectro_net, positive_ver_base);
		printf("pos_ver_max_err: %f\t pos_ver_avr_err: %f\n", pos_ver_max_err, pos_ver_avr_err);
		
		neg_ver_max_err = check_database_max_error(spectro_net, negative_ver_base);
		neg_ver_avr_err = check_database_avg_error(spectro_net, negative_ver_base);
		printf("neg_ver_max_err: %f\t neg_ver_avr_err: %f\n", neg_ver_max_err, neg_ver_avr_err);
		
		printf("\n");
	}
	
	make_database_report(candidates_base, spectro_net, "candidates.xls");
	
	free_database(positive_base);
	free_database(negative_base);
	free_database(positive_ver_base);
	free_database(negative_ver_base);
	free_database(candidates_base);
	
	free_network(spectro_net);
	
	return(1);
}