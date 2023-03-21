#include "network.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

double custom_random(double min, double max)// for stochastic gradient descent
{
	double frand = rand();
	
	frand = (frand/32768)*(max-min) + min;

	return(frand);
}

double sigmoid(double d)
{
	return (1/(1+1/exp(d)));
}

void free_network(network *net)// free reserved by network memory
{
	for(int i=0;i<net->hidden_num;i++)
	{
		free(net->hidden_cell[i]);	
	}
	free(net->out_cell);
	
	free(net);
}

double do_bell(double d)
{
	return(1/exp(d*d));	
}

double get_chaos(double last, double chaos_skew)
{
	if(last<chaos_skew)return(last/chaos_skew);
	else return((1-last)/(1-chaos_skew));
}

cell *init_cell(int inputs_num, int chaos_depth)
{
	int cell_size = sizeof(cell) + sizeof(double) * inputs_num  // synapse_weight
								 + sizeof(double) * chaos_depth // beats
								 + sizeof(double) * chaos_depth // beat_weight_a
								 + sizeof(double) * chaos_depth // beat_weight_b
								 + sizeof(double) * chaos_depth // beat_weight_c
								 + sizeof(double) * chaos_depth // beat_scaled
								 + sizeof(double) * chaos_depth // beat_exp
								 + sizeof(double) * chaos_depth // delta_scale
	;

	cell *neuron = (cell*)malloc(cell_size);
	
	neuron->chaos_depth = chaos_depth;
	neuron->chaos_skew  = //0.618;
						  custom_random(0.1, 0.3);
	neuron->inputs_num  = inputs_num;
	neuron->size        = cell_size;
	neuron->learn_rate  = 0.001;
	
	neuron->synapse_weight = (double*)((char*)neuron + sizeof(cell));
	neuron->beat           = neuron->synapse_weight  + inputs_num;
	neuron->beat_exp       = neuron->beat            + chaos_depth;
	neuron->beat_scaled    = neuron->beat_exp        + chaos_depth;
	neuron->beat_weight_a  = neuron->beat_scaled     + chaos_depth;
	neuron->beat_weight_b  = neuron->beat_weight_a   + chaos_depth;
	neuron->beat_weight_c  = neuron->beat_weight_b   + chaos_depth;
	neuron->delta_scaled   = neuron->beat_weight_c   + chaos_depth;

	
	///////////////////////////////////////////weights init
	neuron->soma_bias = 0.5;//custom_random(-0.1, 0.1);
	for(int i=0;i<inputs_num;i++)
	{
		neuron->synapse_weight[i] = 0;
									//custom_random(-1, 1)/inputs_num;	
	}
	
	neuron->axon_bias = custom_random(-0.1, 0.1);
	for(int i=0;i<chaos_depth;i++)
	{
		neuron->beat_weight_a[i] = //0.1/(i+1);
									//0;
								   custom_random(-1, 1);
		neuron->beat_weight_b[i] = custom_random(0, 1);
		neuron->beat_weight_c[i] = //1;
								   custom_random(-1, 1);
	}
	///////////////////////////////////////////weights init
	
	return(neuron);
}

network *init_network(int inputs_num, int hidden_num, int chaos_depth)
{
	int net_size = sizeof(network) + sizeof(cell*) * hidden_num
								   + sizeof(double) * hidden_num // store outputs of hidden layer
	;
	
	network *net = (network*)malloc(net_size);
	
	net->hidden_num = hidden_num;
	net->inputs_num = inputs_num;
	net->size       = net_size;
	
	net->hidden_cell = (cell**)((char*)net + sizeof(network));
	net->hidden_out = (double*)((char*)net->hidden_cell + sizeof(cell*)*hidden_num);
	
	/////////////////////////make random seed
	static int first = 1;
	if(first)
	{
		srand(time(NULL));
		first = 0;
	}
	/////////////////////////make random seed
	
	for(int i=0;i<hidden_num;i++)
	{
		net->hidden_cell[i] = init_cell(inputs_num, chaos_depth);	
	}
	net->out_cell = init_cell(hidden_num, chaos_depth);
	
	return(net);
}

double run_cell(cell *neuron, double *input)
{
	neuron->soma_sum = neuron->soma_bias;
	for(int i=0;i<neuron->inputs_num;i++)
	{
		neuron->soma_sum += input[i] * neuron->synapse_weight[i];
	}
	
	////////////////////////////////////axon
	neuron->axon_out = neuron->axon_bias;
	
	neuron->beat[0]        = neuron->soma_sum;
	neuron->beat_scaled[0] = (neuron->beat[0] * neuron->beat_weight_a[0]) + neuron->beat_weight_b[0];
	neuron->beat_exp[0]    = neuron->beat_weight_c[0] * do_bell(neuron->beat_scaled[0]);
	
	neuron->axon_out += neuron->beat_exp[0];

	
	for(int i=1;i<neuron->chaos_depth;i++)
	{
		neuron->beat[i]        = get_chaos(neuron->beat[i-1],neuron->chaos_skew);
		neuron->beat_scaled[i] = (neuron->beat[i] * neuron->beat_weight_a[i]) + neuron->beat_weight_b[i];
		neuron->beat_exp[i]    = neuron->beat_weight_c[i] * do_bell(neuron->beat_scaled[i]);
		
		neuron->axon_out += neuron->beat_exp[i];
	}
	////////////////////////////////////axon
	
	return(neuron->axon_out);
}

double run_output_cell(cell *neuron, double *input)
{
	neuron->soma_sum = neuron->soma_bias;
	for(int i=0;i<neuron->inputs_num;i++)
	{
		neuron->soma_sum += input[i] * neuron->synapse_weight[i];	
	}
	
	//neuron->axon_out = neuron->soma_sum;
	neuron->axon_out = sigmoid(neuron->soma_sum);
	
	return(neuron->axon_out);
}

double run_network(network *net, double *input)
{
	for(int i=0;i<net->hidden_num;i++)
	{
		net->hidden_out[i] = run_cell(net->hidden_cell[i], input);
	}
	net->out = run_output_cell(net->out_cell, net->hidden_out);
	//net->out = run_cell(net->out_cell, net->hidden_out);
	
	return(net->out);
}

double train_output_cell(cell *neuron, double *train_input, double deep_delta)
{
	//neuron->soma_bias *= 0.99999;
	//deep_delta *= neuron->axon_out*(1-neuron->axon_out);
	
	deep_delta *= neuron->axon_out*(1-neuron->axon_out);
	
	neuron->soma_bias -= neuron->learn_rate * deep_delta;
	for(int i=0;i<neuron->inputs_num;i++)
	{
		//neuron->synapse_weight[i] *= 0.99999;
		neuron->synapse_weight[i] -= neuron->learn_rate * deep_delta * train_input[i];
	}
	return(deep_delta);
}

double train_cell(cell *neuron, double *train_input, double deep_delta)
{
	for(int i=0;i<neuron->chaos_depth;i++)
	{
		neuron->delta_scaled[i] = deep_delta * (-2 * neuron->beat_exp[i] * neuron->beat_scaled[i]);// dE/dScaled
	}
	
	double delta_soma_sum;//dE/dSoma_sum
	double beats_deriv=1;
	
	delta_soma_sum = neuron->delta_scaled[0] * neuron->beat_weight_a[0];//x0
	
	for(int i=1;i<neuron->chaos_depth;i++)
	{	
		if(neuron->beat[i-1]<neuron->chaos_skew)beats_deriv /= neuron->chaos_skew;
		else beats_deriv /= neuron->chaos_skew-1;
		
		delta_soma_sum += beats_deriv * neuron->delta_scaled[i] * neuron->beat_weight_a[i]; 
	}
	
	///////////////////////////////////////////////////////axon correction

	
	neuron->axon_bias -= deep_delta * neuron->learn_rate;
	for(int i=0;i<neuron->chaos_depth;i++)
	{
		neuron->beat_weight_c[i] -= deep_delta * do_bell(neuron->beat_scaled[i]) * neuron->learn_rate;
		neuron->beat_weight_b[i] -= neuron->delta_scaled[i] * neuron->learn_rate;
		neuron->beat_weight_a[i] -= neuron->beat[i] * neuron->delta_scaled[i] * neuron->learn_rate;
	}
	///////////////////////////////////////////////////////axon correction
	
	///////////////////////////////////////////////////////synapses correction
	//neuron->soma_bias *= 0.99999;
	neuron->soma_bias -=  delta_soma_sum * neuron->learn_rate;	
	for(int i=0;i<neuron->inputs_num;i++)
	{
		//neuron->synapse_weight[i] *= 0.99999;
		neuron->synapse_weight[i] -= delta_soma_sum * train_input[i] * neuron->learn_rate;		
	}
	///////////////////////////////////////////////////////synapses correction
	
	return(delta_soma_sum);
}

void train_network_pattern(network *net, double *train_input, double *train_output)
{
	double deep_delta = run_network(net, train_input) - train_output[0];//dE/dout
	
	deep_delta = train_output_cell(net->out_cell, net->hidden_out, deep_delta);
	//deep_delta = train_cell(net->out_cell, net->hidden_out, deep_delta);
	
	for(int i=0;i<net->hidden_num;i++)
	{
		train_cell(net->hidden_cell[i], train_input, deep_delta * net->out_cell->synapse_weight[i]);	
	}
}