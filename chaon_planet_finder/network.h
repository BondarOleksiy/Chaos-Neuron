typedef struct cell
{
	int size;
	
	int inputs_num;
	int chaos_depth;
	double chaos_skew;
	double learn_rate;
	
	double *synapse_weight;
	double soma_bias;
	
	double soma_sum;
	
	double *beat;
	double *beat_weight_a;
	double *beat_weight_b;
	double *beat_weight_c;
	
	double *beat_scaled;
	double *beat_exp;
	double *delta_scaled;
	
	double axon_bias;
	
	double axon_out;
	
}cell;

typedef struct network
{
	int size;
	
	int inputs_num;
	
	int hidden_num;

	cell **hidden_cell;
	double *hidden_out;
	cell  *out_cell;
	
	double out;
	
}network;

void free_network(network *net);
double run_network(network *net, double *input);
void train_network_pattern(network *net, double *train_input, double *train_output);
network *init_network(int inputs_num, int hidden_num, int chaos_depth);