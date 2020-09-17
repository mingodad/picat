#ifdef FANN

extern "C" {
#include "../picat.h"
#include "../picat_utilities.h"
}

#include "floatfann.h"
#include <cstring>

#include <map>

using namespace std;

unsigned int n_id = 0;
unsigned int t_id = 0;

//fann *ann;
map<unsigned int, fann*> anns;
map<unsigned int, fann_train_data*> training_data;

#define PICAT_GET_FLOAT(x) NUMVAL(x)

int picat_array_to_int_array(TERM ta, int *a, int size) {
	for (int i = 0; i < picat_get_struct_arity(ta); i++) {
		a[i] = picat_get_integer(picat_get_arg(i + 1, ta));
	}

	return PICAT_TRUE;
}

int picat_array_to_uint_array(TERM ta, unsigned int *a) {
	for (int i = 0; i < picat_get_struct_arity(ta); i++) {
		a[i] = picat_get_integer(picat_get_arg(i + 1, ta));
	}

	return PICAT_TRUE;
}

int picat_array_to_fann_array(TERM ta, fann_type *a) {
	for (int i = 0; i < picat_get_struct_arity(ta); i++) {
	  TERM num = picat_get_arg(i + 1, ta);
	  DEREF(num);
	  a[i] = PICAT_GET_FLOAT(num);
	}

	return PICAT_TRUE;
}

int fann_to_picat_array(fann_type *a, int num, TERM& ta) {
	TERM tmp = picat_build_structure((char*)"{}", num);
	for (int j = 1; j <= num; j++) {
		TERM temp = picat_get_arg(j, tmp);

		picat_unify(temp, picat_build_float(a[j - 1]));
	}

	return picat_unify(ta, tmp);
}

int unsigned_int_array_to_picat_array(unsigned int  *a, int num, TERM& ta) {
  TERM tmp = picat_build_structure((char*)"{}", num);
  for (int j = 1; j <= num; j++) {
	TERM temp = picat_get_arg(j, tmp);

	picat_unify(temp, picat_build_integer(a[j - 1]));
  }

  return picat_unify(ta, tmp);
}

fann_activationfunc_enum picat_to_fann_func(TERM f) {
	char* func = picat_get_atom_name(f);

	if (!strcmp(func,"FANN_LINEAR")) {
		return FANN_LINEAR;
	}
	else if (!strcmp(func,"FANN_THRESHOLD")) {
		return FANN_THRESHOLD;
	}
	else if (!strcmp(func,"FANN_THRESHOLD_SYMMETRIC")) {
		return FANN_THRESHOLD_SYMMETRIC;
	}
	else if (!strcmp(func,"FANN_SIGMOID")) {
		return FANN_SIGMOID;
	}
	else if (!strcmp(func,"FANN_SIGMOID_STEPWISE")) {
		return FANN_SIGMOID_STEPWISE;
	}
	else if (!strcmp(func,"FANN_SIGMOID_SYMMETRIC")) {
		return FANN_SIGMOID_SYMMETRIC;
	}
	else if (!strcmp(func,"FANN_SIGMOID_SYMMETRIC_STEPWISE")) {
		return FANN_SIGMOID_SYMMETRIC_STEPWISE;
	}
	else if (!strcmp(func,"FANN_GAUSSIAN")) {
		return FANN_GAUSSIAN;
	}
	else if (!strcmp(func,"FANN_GAUSSIAN_SYMMETRIC")) {
		return FANN_GAUSSIAN_SYMMETRIC;
	}
	else if (!strcmp(func,"FANN_GAUSSIAN_STEPWISE")) {
		return FANN_GAUSSIAN_STEPWISE;
	}
	else if (!strcmp(func,"FANN_ELLIOT")) {
		return FANN_ELLIOT;
	}
	else if (!strcmp(func,"FANN_ELLIOT_SYMMETRIC")) {
		return FANN_ELLIOT_SYMMETRIC;
	}
	else if (!strcmp(func,"FANN_LINEAR_PIECE")) {
		return FANN_LINEAR_PIECE;
	}
	else if (!strcmp(func,"FANN_LINEAR_PIECE_SYMMETRIC")) {
		return FANN_LINEAR_PIECE_SYMMETRIC;
	}
	else if (!strcmp(func,"FANN_SIN_SYMMETRIC")) {
		return FANN_SIN_SYMMETRIC;
	}
	else if (!strcmp(func,"FANN_COS_SYMMETRIC")) {
		return FANN_COS_SYMMETRIC;
	}
	else if (!strcmp(func,"FANN_SIN")) {
		return FANN_SIN;
	}
	else if (!strcmp(func,"FANN_COS")) {
		return FANN_COS;
	}
}

extern "C" {
	int fann_destroy_all() {
		for (auto const& x : training_data){
			fann_destroy_train(x.second);
		}
		for (auto const& x : anns) {
			fann_destroy(x.second);
		}
		n_id = 0;
		t_id = 0;

		return PICAT_TRUE;
	}



	int pi_get_data_in_out() {
		TERM d = picat_get_call_arg(1, 4);
		TERM i = picat_get_call_arg(2, 4);
		TERM in = picat_get_call_arg(3, 4);
		TERM out = picat_get_call_arg(4, 4);

		TERM d_id = picat_get_arg(1, d);

		int id = picat_get_integer(d_id);
		int index = picat_get_integer(i);

		fann_train_data *data = training_data[picat_get_integer(d_id)];
	
		return fann_to_picat_array(data->input[index], data->num_input, in) * fann_to_picat_array(data->output[index], data->num_output, out);
	}

	int pi_fann_create_standard() {
		TERM layer_sizes = picat_get_call_arg(1, 2);
		TERM ret = picat_get_call_arg(2, 2);
		
		int num_layers = picat_get_struct_arity(layer_sizes);
		unsigned int *layers = (unsigned int *)calloc(num_layers, sizeof(unsigned int));
		picat_array_to_uint_array(layer_sizes,layers);

		if (num_layers)
		{
			anns[n_id] = fann_create_standard_array(num_layers, layers);
			free(layers);

			TERM temp = picat_build_structure((char*)"fann", 1);
			TERM id = picat_get_arg(1, temp);
			picat_unify(id, picat_build_integer(n_id));

			n_id++;

			return picat_unify(ret, temp);
		}
		else
		{
			fann_error(NULL, FANN_E_CANT_ALLOCATE_MEM);
			return PICAT_FALSE;
		}
	}

	int pi_fann_create_sparse() {
		TERM c_rate = picat_get_call_arg(1, 3);
		TERM layer_sizes = picat_get_call_arg(2, 3);
		TERM ret = picat_get_call_arg(3, 3);

		float con_rate = PICAT_GET_FLOAT(c_rate);

		int num_layers = picat_get_struct_arity(layer_sizes);
		unsigned int *layers = (unsigned int *)calloc(num_layers, sizeof(unsigned int));;
		picat_array_to_uint_array(layer_sizes, layers);

		if (num_layers) {
			anns[n_id] = fann_create_sparse_array(con_rate, num_layers, layers);
			free(layers);

			TERM temp = picat_build_structure((char*)"fann", 1);
			TERM id = picat_get_arg(1, temp);
			picat_unify(id, picat_build_integer(n_id));

			n_id++;

			return picat_unify(ret, temp);
		}
		else
		{
			fann_error(NULL, FANN_E_CANT_ALLOCATE_MEM);
			return PICAT_FALSE;
		}
	}

	int pi_fann_create_shortcut() {
		TERM layer_sizes = picat_get_call_arg(1, 2);
		TERM ret = picat_get_call_arg(2, 2);

		int num_layers = picat_get_struct_arity(layer_sizes);
		unsigned int *layers = (unsigned int *)calloc(num_layers, sizeof(unsigned int));;
		picat_array_to_uint_array(layer_sizes, layers);

		if (num_layers) {
		    anns[n_id] = fann_create_shortcut_array(num_layers, layers);
			free(layers);

			TERM temp = picat_build_structure((char*)"fann", 1);
			TERM id = picat_get_arg(1, temp);
			picat_unify(id, picat_build_integer(n_id));

			n_id++;

			return picat_unify(ret, temp);
		}
		else
		{
			fann_error(NULL, FANN_E_CANT_ALLOCATE_MEM);
			return PICAT_FALSE;
		}
	}

	int pi_fann_destroy() {
		TERM nn = picat_get_call_arg(1, 1);
		TERM id = picat_get_arg(1, nn);

		fann_destroy(anns[picat_get_integer(id)]);
		return PICAT_TRUE;
	}

	int pi_fann_copy() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM ret = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		anns[n_id] = fann_copy(ann);


		TERM temp = picat_build_structure((char*)"fann", 1);
		TERM id = picat_get_arg(1, temp);
		picat_unify(id, picat_build_integer(n_id));

		n_id++;

		return picat_unify(ret, temp);
	}

	int pi_fann_run() {
		TERM nn = picat_get_call_arg(1, 3);
		TERM in = picat_get_call_arg(2, 3);
		TERM ret = picat_get_call_arg(3, 3);

		TERM id = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(id)];

		fann_type *input = (fann_type *)calloc(fann_get_num_input(ann), sizeof(fann_type));
		picat_array_to_fann_array(in, input);

		int out_num = fann_get_num_output(ann);
		fann_type *temp = (fann_type *)calloc(out_num, sizeof(fann_type)); 
		temp = fann_run(ann, input);


		return fann_to_picat_array(temp, out_num, ret);
	}

	int pi_fann_randomize_weights() {
		TERM nn = picat_get_call_arg(1, 3);
		TERM min = picat_get_call_arg(2, 3); DEREF(min);
		TERM max = picat_get_call_arg(3, 3); DEREF(max);

		TERM id = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(id)];

		fann_type min_weight =  PICAT_GET_FLOAT(min);
		fann_type max_weight = PICAT_GET_FLOAT(max);

		fann_randomize_weights(ann, min_weight, max_weight);

		return PICAT_TRUE;
	}

	int pi_fann_init_weights() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM data = picat_get_call_arg(2, 2);
		
		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		TERM did = picat_get_arg(1, data);
		fann_init_weights(ann, training_data[picat_get_integer(did)]);

		return PICAT_TRUE;
	}

	int pi_fann_print_connections() {
		TERM nn = picat_get_call_arg(1, 1);
		TERM nid = picat_get_arg(1, nn);

		fann* ann = anns[picat_get_integer(nid)];

		fann_print_connections(ann);

		return PICAT_TRUE;
	}

	int pi_fann_print_parameters() {
		TERM nn = picat_get_call_arg(1, 1);
		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		fann_print_parameters(ann);

		return PICAT_TRUE;
	}

	int pi_fann_get_num_input() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM ret = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		TERM temp = picat_build_integer(fann_get_num_input(ann));

		return picat_unify(ret, temp);
	}

	int pi_fann_get_num_output() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM ret = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		TERM temp = picat_build_integer(fann_get_num_output(ann));

		return picat_unify(ret, temp);
	}

	int pi_fann_get_total_neurons() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM ret = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		TERM temp = picat_build_integer(fann_get_total_neurons(ann));

		return picat_unify(ret, temp);
	}

	int pi_fann_get_total_connections() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM ret = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		TERM temp = picat_build_integer(fann_get_total_connections(ann));

		return picat_unify(ret, temp);
	}

	int pi_fann_get_network_type() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM ret = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		enum fann_nettype_enum type = fann_get_network_type(ann);
		if (type == FANN_NETTYPE_LAYER)
			return picat_unify(ret, picat_build_atom("FANN_NETTYPE_LAYER"));
		else
			return picat_unify(ret, picat_build_atom("FANN_NETTYPE_SHORTCUT"));

	}

	int pi_fann_get_connection_rate() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM ret = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		float rate = fann_get_connection_rate(ann);

		return picat_unify(ret, picat_build_float(rate));
	}

	int pi_fann_get_num_layers() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM ret = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		int temp = fann_get_num_layers(ann);

		return picat_unify(ret, picat_build_integer(temp));
	}

  	/*

	void fann_get_bias_array(struct fann *ann, unsigned int *bias);

	void fann_get_connection_array(struct fann *ann, struct fann_connection *connections);


	void fann_set_weight_array(struct fann *ann, struct fann_connection *connections, unsigned int num_connections);

	void fann_set_weight(struct fann *ann, unsigned int from_neuron, unsigned int to_neuron, fann_type weight);

	void fann_set_user_data(struct fann *ann, void *user_data);

	void * fann_get_user_data(struct fann *ann);
	*/



	int pi_fann_train() {
		TERM nn = picat_get_call_arg(1, 3);
		TERM in = picat_get_call_arg(2, 3);
		TERM out = picat_get_call_arg(3, 3);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		fann_type *input = (fann_type *)calloc(fann_get_num_input(ann), sizeof(unsigned int)); 
		fann_type *desired_output = (fann_type *)calloc(fann_get_num_output(ann), sizeof(unsigned int));

		picat_array_to_fann_array(in, input);
		picat_array_to_fann_array(out, desired_output);

		fann_train(ann, input, desired_output);

		return PICAT_TRUE;
	}

	int pi_fann_test() {
		TERM nn = picat_get_call_arg(1, 4);
		TERM in = picat_get_call_arg(2, 4);
		TERM out = picat_get_call_arg(3, 4);
		TERM ret = picat_get_call_arg(4, 4);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		fann_type *input = (fann_type *)calloc(fann_get_num_input(ann), sizeof(fann_type));
		fann_type *desired_output = (fann_type *)calloc(fann_get_num_output(ann), sizeof(fann_type));

		picat_array_to_fann_array(in, input);
		picat_array_to_fann_array(out, desired_output);

		fann_type temp = *fann_test(ann, input, desired_output);

		return picat_unify(ret, picat_build_float(temp));
	}

	int pi_fann_get_MSE() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM ret = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		float temp = fann_get_MSE(ann);

		return picat_unify(ret, picat_build_float(temp));
	}

	int pi_fann_get_bit_fail() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM ret = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		int temp = fann_get_bit_fail(ann);

		return picat_unify(ret, picat_build_integer(temp));

	}

	int pi_fann_reset_MSE() {
		TERM nn = picat_get_call_arg(1, 1);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		fann_reset_MSE(ann);

		return PICAT_TRUE;
	}


	int pi_fann_train_on_data() {
		TERM nn = picat_get_call_arg(1, 5);
		TERM d = picat_get_call_arg(2, 5);
		TERM max_e = picat_get_call_arg(3, 5);
		TERM ep_bet_rep = picat_get_call_arg(4, 5);
		TERM d_err = picat_get_call_arg(5, 5); DEREF(d_err);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		TERM did = picat_get_arg(1, d);
		fann_train_data *data = training_data[picat_get_integer(did)]; 

		int max_epochs = picat_get_integer(max_e);
		int epochs_between_reports = picat_get_integer(ep_bet_rep);
		float desired_error = PICAT_GET_FLOAT(d_err);

		fann_train_on_data(ann, data, max_epochs, epochs_between_reports, desired_error);

		return PICAT_TRUE;
	}

	int pi_fann_train_on_file() {
		TERM nn = picat_get_call_arg(1, 5);
		TERM fn = picat_get_call_arg(2, 5);
		TERM max_e = picat_get_call_arg(3, 5);
		TERM ep_bet_rep = picat_get_call_arg(4, 5);
		TERM d_err = picat_get_call_arg(5, 5); DEREF(d_err);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		char* filename = picat_string_to_cstring(fn);
		int max_epochs = picat_get_integer(max_e);
		int epochs_between_reports = picat_get_integer(ep_bet_rep);
		float desired_error = PICAT_GET_FLOAT(d_err);

		fann_train_on_file(ann, filename, max_epochs, epochs_between_reports, desired_error);

		return PICAT_TRUE;
	}

	int pi_fann_train_epoch() {
		TERM nn = picat_get_call_arg(1, 3);
		TERM d = picat_get_call_arg(2, 3);
		TERM ret = picat_get_call_arg(3, 3);
		
		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		TERM did = picat_get_arg(1, d);
		fann_train_data *data = training_data[picat_get_integer(did)]; 

		float temp = fann_train_epoch(ann, data);

		return picat_unify(ret, picat_build_float(temp));
	}

	// fann_test_data

	int pi_fann_read_train_from_file() {
		TERM fn, ret;

		fn = picat_get_call_arg(1, 2);
		ret = picat_get_call_arg(2, 2);

		char* filename = picat_string_to_cstring(fn);

		training_data[t_id] = fann_read_train_from_file(filename);


		TERM temp = picat_build_structure((char*)"training_data", 1);
		TERM id = picat_get_arg(1, temp);
		picat_unify(id, picat_build_integer(t_id));

		t_id++;

		return picat_unify(ret, temp);
	}


	int pi_fann_create_train() {
		TERM num_d, num_i, num_o, ret;
		num_d = picat_get_call_arg(1, 4);
		num_i = picat_get_call_arg(2, 4);
		num_o = picat_get_call_arg(3, 4);
		ret = picat_get_call_arg(4, 4);


		int num_data = picat_get_integer(num_d);
		int num_input = picat_get_integer(num_i);
		int num_output = picat_get_integer(num_o);

		training_data[t_id] = fann_create_train(num_data, num_input, num_output);


		TERM temp = picat_build_structure((char*)"training_data", 1);
		TERM id = picat_get_arg(1, temp);
		picat_unify(id, picat_build_integer(t_id));

		t_id++;

		return picat_unify(ret, temp);
	}

	
	int pi_fann_create_train_array() {
		TERM num_d = picat_get_call_arg(1, 6);
		TERM num_i = picat_get_call_arg(2, 6);
		TERM in = picat_get_call_arg(3, 6);
		TERM num_o = picat_get_call_arg(4, 6);
		TERM out = picat_get_call_arg(5, 6);
		TERM ret = picat_get_call_arg(6, 6);


		int num_data = picat_get_integer(num_d);
		int num_input = picat_get_integer(num_i);
		int num_output = picat_get_integer(num_o);

		fann_type *input_vector = (fann_type *)calloc(num_data, sizeof(fann_type));
		fann_type *output_vector = (fann_type *)calloc(num_data, sizeof(fann_type));


		training_data[t_id] = fann_create_train_array(num_data, num_input, input_vector, num_output, output_vector);


		TERM temp = picat_build_structure((char*)"training_data", 1);
		TERM id = picat_get_arg(1, temp);
		picat_unify(id, picat_build_integer(t_id));

		t_id++;

		return picat_unify(ret, temp);
	}


	/*
	struct fann_train_data * fann_create_train_from_callback(unsigned int num_data,
		unsigned int num_input,
		unsigned int num_output,
		void (*user_function)(unsigned int,
			unsigned int,
			unsigned int,
			fann_type *,
			fann_type *));
	*/


	int pi_fann_destroy_train() {
		TERM data = picat_get_call_arg(1, 1);
		TERM id = picat_get_arg(1, data);

		fann_destroy_train(training_data[picat_get_integer(id)]);

		training_data.erase(picat_get_integer(id));

		return PICAT_TRUE;
	}


	int pi_fann_shuffle_train_data() {
		TERM data = picat_get_call_arg(1, 1);
		TERM id = picat_get_arg(1, data);

		fann_shuffle_train_data(training_data[picat_get_integer(id)]);

		return PICAT_TRUE;
	}

	int pi_fann_scale_train() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM data = picat_get_call_arg(2, 2);
		
		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		TERM did = picat_get_arg(1, data);
		fann_train_data* train = training_data[picat_get_integer(did)];

		fann_scale_train(ann, train);

		return PICAT_TRUE;
	}

	int pi_fann_descale_train() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM data = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		TERM id = picat_get_arg(1, data);

		fann_descale_train(ann, training_data[picat_get_integer(id)]);

		return PICAT_TRUE;
	}

	int pi_fann_set_input_scaling_params() {
		TERM nn = picat_get_call_arg(1, 5);
		TERM data = picat_get_call_arg(2, 5);
		TERM min = picat_get_call_arg(3, 5); DEREF(min);
		TERM max = picat_get_call_arg(4, 5); DEREF(max);
		TERM ret = picat_get_call_arg(5, 5);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		TERM did = picat_get_arg(1, data);
		fann_train_data* t_data = training_data[picat_get_integer(did)];

		float new_input_min = PICAT_GET_FLOAT(min);
		float new_input_max = PICAT_GET_FLOAT(max);

		int temp = fann_set_input_scaling_params(ann, t_data, new_input_min, new_input_max);

		return picat_unify(ret, picat_build_integer(temp));
	}

	int pi_fann_set_output_scaling_params() {
		TERM nn = picat_get_call_arg(1, 5);
		TERM data = picat_get_call_arg(2, 5);
		TERM min = picat_get_call_arg(3, 5); DEREF(min);
		TERM max = picat_get_call_arg(4, 5); DEREF(max);
		TERM ret = picat_get_call_arg(5, 5);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		TERM did = picat_get_arg(1, data);
		fann_train_data* t_data = training_data[picat_get_integer(did)];

		float new_output_min = PICAT_GET_FLOAT(min);
		float new_output_max = PICAT_GET_FLOAT(max);

		int temp = fann_set_output_scaling_params(ann, t_data, new_output_min, new_output_max);

		return picat_unify(ret, picat_build_integer(temp));
	}

	int pi_fann_set_scaling_params() {
		TERM nn		= picat_get_call_arg(1, 7);
		TERM data	= picat_get_call_arg(2, 7);
		TERM i_min	= picat_get_call_arg(3, 7);
		TERM i_max	= picat_get_call_arg(4, 7);
		TERM o_min	= picat_get_call_arg(5, 7);
		TERM o_max	= picat_get_call_arg(6, 7);
		TERM ret	= picat_get_call_arg(7, 7);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		TERM did = picat_get_arg(1, data);
		fann_train_data* t_data = training_data[picat_get_integer(did)];

		float new_input_min = picat_get_integer(i_min);
		float new_input_max = picat_get_integer(i_max);
		float new_output_min = picat_get_integer(o_min);
		float new_output_max = picat_get_integer(o_max);

		//		printf("scaling %f %f %f %f \n", new_input_min, new_input_max, new_output_min, new_output_max);
		
		int temp = fann_set_scaling_params(ann, t_data, new_input_min, new_input_max, new_output_min, new_output_max);

		return picat_unify(ret, picat_build_integer(temp));
	}

	int pi_fann_clear_scaling_params() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM ret = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		int temp = fann_clear_scaling_params(ann);

		return picat_unify(ret, picat_build_integer(temp));

	}

	int pi_fann_scale_input() {
		TERM nn = picat_get_call_arg(1, 3);
		TERM in = picat_get_call_arg(2, 3);
		TERM ret = picat_get_call_arg(3,3);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		fann_type *input_vector = (fann_type *)calloc(fann_get_num_input(ann), sizeof(unsigned int));

		picat_array_to_fann_array(in, input_vector);

		fann_scale_input(ann, input_vector);

		return fann_to_picat_array(input_vector, fann_get_num_input(ann), ret);
	}

	int pi_fann_scale_output() {	
		TERM nn = picat_get_call_arg(1, 3);
		TERM out = picat_get_call_arg(2, 3);
		TERM ret = picat_get_call_arg(3, 3);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		fann_type *output_vector = (fann_type *)calloc(fann_get_num_output(ann), sizeof(unsigned int));

		picat_array_to_fann_array(out, output_vector);

		fann_scale_output(ann, output_vector);

		return fann_to_picat_array(output_vector, fann_get_num_output(ann), ret);

	}


	int pi_fann_descale_input() {
		TERM nn = picat_get_call_arg(1, 3);
		TERM in = picat_get_call_arg(1, 3);
		TERM ret = picat_get_call_arg(3, 3);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		fann_type *input_vector = (fann_type *)calloc(fann_get_num_input(ann), sizeof(unsigned int));

		picat_array_to_fann_array(in, input_vector);

		fann_descale_input(ann, input_vector);

		return fann_to_picat_array(input_vector, fann_get_num_input(ann), ret);
	}

	int pi_fann_descale_output() {
		TERM nn = picat_get_call_arg(1, 3);
		TERM out = picat_get_call_arg(2, 3);
		TERM ret = picat_get_call_arg(3, 3);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		fann_type *output_vector = (fann_type *)calloc(fann_get_num_output(ann), sizeof(unsigned int));

		picat_array_to_fann_array(out, output_vector);

		fann_descale_output(ann, output_vector);

		return fann_to_picat_array(output_vector, fann_get_num_output(ann), ret);
	}


	int pi_fann_scale_input_train_data() {
		TERM data = picat_get_call_arg(1, 3);
		TERM id = picat_get_arg(1, data);
		fann_train_data* t_data = training_data[picat_get_integer(id)];

		TERM min = picat_get_call_arg(2, 3); DEREF(min);
		TERM max = picat_get_call_arg(3, 3); DEREF(max);

		float new_min = PICAT_GET_FLOAT(min);
		float new_max = PICAT_GET_FLOAT(max);

		fann_scale_input_train_data(t_data, new_min, new_max);

		return PICAT_TRUE;
	}


	int pi_fann_scale_output_train_data() {
		TERM data = picat_get_call_arg(1, 3);
		TERM id = picat_get_arg(1, data);
		fann_train_data* t_data = training_data[picat_get_integer(id)];

		TERM min = picat_get_call_arg(2, 3); DEREF(min);
		TERM max = picat_get_call_arg(3, 3); DEREF(max);

		float new_min = PICAT_GET_FLOAT(min);
		float new_max = PICAT_GET_FLOAT(max);

		fann_scale_output_train_data(t_data, new_min, new_max);

		return PICAT_TRUE;
	}

	int pi_fann_scale_train_data() {
		TERM data = picat_get_call_arg(1, 3);
		TERM id = picat_get_arg(1, data);
		fann_train_data* t_data = training_data[picat_get_integer(id)];

		TERM min = picat_get_call_arg(2, 3); DEREF(min);
		TERM max = picat_get_call_arg(3, 3); DEREF(max);

		float new_min = PICAT_GET_FLOAT(min);
		float new_max = PICAT_GET_FLOAT(max);

		fann_scale_train_data(t_data, new_min, new_max);

		return PICAT_TRUE;
	}

	int pi_fann_merge_train_data() {
		TERM data1 = picat_get_call_arg(1, 3);
		TERM id1 = picat_get_arg(1, data1);
		fann_train_data* t_data1 = training_data[picat_get_integer(id1)];

		TERM data2 = picat_get_call_arg(2, 3);
		TERM id2 = picat_get_arg(1, data2);
		fann_train_data* t_data2 = training_data[picat_get_integer(id2)];

		TERM ret = picat_get_call_arg(3, 3);

		training_data[t_id] = fann_merge_train_data(t_data1, t_data2);

		TERM temp = picat_build_structure((char*)"training_data", 1);
		TERM id = picat_get_arg(1, temp);
		picat_unify(id, picat_build_integer(t_id));

		t_id++;

		return picat_unify(ret, temp);
	}

	int pi_fann_duplicate_train_data() {
		TERM data = picat_get_call_arg(1, 2);
		TERM id = picat_get_arg(1, data);
		fann_train_data* t_data = training_data[picat_get_integer(id)];

		TERM ret = picat_get_call_arg(2, 2);

		training_data[t_id] = fann_duplicate_train_data(t_data);

		TERM temp = picat_build_structure((char*)"training_data", 1);
		TERM id_num = picat_get_arg(1, temp);
		picat_unify(id_num, picat_build_integer(t_id));

		t_id++;

		return picat_unify(ret, temp);
	}

	int pi_fann_subset_train_data() {
		TERM data = picat_get_call_arg(1, 4);
		TERM id = picat_get_arg(1, data);
		fann_train_data* t_data = training_data[picat_get_integer(id)];

		TERM p = picat_get_call_arg(2, 4);
		TERM l = picat_get_call_arg(3, 4);

		TERM ret = picat_get_call_arg(4, 4);

		int pos = picat_get_integer(p);
		int length = picat_get_integer(l);

		training_data[t_id] = fann_subset_train_data(t_data, pos, length);

		TERM temp = picat_build_structure((char*)"training_data", 1);
		TERM id_num = picat_get_arg(1, temp);
		picat_unify(id_num, picat_build_integer(t_id));

		t_id++;

		return picat_unify(ret, temp);
	}

	int pi_fann_length_train_data() {
		TERM data = picat_get_call_arg(1, 2);
		TERM id = picat_get_arg(1, data);
		fann_train_data* t_data = training_data[picat_get_integer(id)];

		TERM ret = picat_get_call_arg(2, 2);

		unsigned int temp = fann_length_train_data(t_data);

		return picat_unify(ret, picat_build_integer(temp));
	}

	int pi_fann_num_input_train_data() {
		TERM data = picat_get_call_arg(1, 2);
		TERM id = picat_get_arg(1, data);
		fann_train_data* t_data = training_data[picat_get_integer(id)];

		TERM ret = picat_get_call_arg(2, 2);

		unsigned int temp = fann_num_input_train_data(t_data);

		return picat_unify(ret, picat_build_integer(temp));
	}

	int pi_fann_num_output_train_data() {
		TERM data = picat_get_call_arg(1, 2);
		TERM id = picat_get_arg(1, data);
		fann_train_data* t_data = training_data[picat_get_integer(id)];

		TERM ret = picat_get_call_arg(2, 2);

		unsigned int temp = fann_num_output_train_data(t_data);

		return picat_unify(ret, picat_build_integer(temp));
	}


	int pi_fann_save_train() {
		TERM data = picat_get_call_arg(1, 3);
		TERM id = picat_get_arg(1, data);
		fann_train_data* t_data = training_data[picat_get_integer(id)];

		TERM f_name = picat_get_call_arg(2, 3);
		char* filename = picat_string_to_cstring(f_name);

		TERM ret = picat_get_call_arg(3, 3);

		unsigned int temp = fann_save_train(t_data, filename);

		return picat_unify(ret, picat_build_integer(temp));
	}


	int pi_fann_save_train_to_fixed() {
		TERM data = picat_get_call_arg(1, 4);
		TERM id = picat_get_arg(1, data);
		fann_train_data* t_data = training_data[picat_get_integer(id)];

		TERM f_name = picat_get_call_arg(2, 4);
		char* filename = picat_string_to_cstring(f_name);

		TERM dec = picat_get_call_arg(3, 4);
		int decimal_point = picat_get_integer(dec);

		TERM ret = picat_get_call_arg(4, 4);

		unsigned int temp = fann_save_train_to_fixed(t_data, filename, decimal_point);

		return picat_unify(ret, picat_build_integer(temp));
	}



	int pi_fann_get_training_algorithm() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM ret = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		fann_train_enum temp = fann_get_training_algorithm(ann);

		switch (temp) {
		case 0:
			return picat_unify(ret, picat_build_atom("FANN_TRAIN_INCREMENTAL"));
		case 1:
			return picat_unify(ret, picat_build_atom("FANN_TRAIN_BATCH"));
		case 2:
			return picat_unify(ret, picat_build_atom("FANN_TRAIN_RPROP"));
		case 3:
			return picat_unify(ret, picat_build_atom("FANN_TRAIN_QUICKPROP"));
		case 4:
			return picat_unify(ret, picat_build_atom("FANN_TRAIN_SARPROP"));
		}
	}

	int pi_fann_set_training_algorithm() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM alg = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		char* temp = picat_get_atom_name(alg);

		fann_train_enum training_algorithm = FANN_TRAIN_INCREMENTAL;
		if (!strcmp(temp, "FANN_TRAIN_BATCH")) {
			training_algorithm = FANN_TRAIN_BATCH;
		}
		else if (!strcmp(temp, "FANN_TRAIN_RPROP")) {
			training_algorithm = FANN_TRAIN_RPROP;
		}
		else if (!strcmp(temp, "FANN_TRAIN_QUICKPROP")) {
			training_algorithm = FANN_TRAIN_QUICKPROP;
		}
		else if (!strcmp(temp, "FANN_TRAIN_SARPROP")) {
			training_algorithm = FANN_TRAIN_SARPROP;
		}

		fann_set_training_algorithm(ann, training_algorithm);

		return PICAT_TRUE;
	}

	int pi_fann_get_learning_rate() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM ret = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		float temp = fann_get_learning_rate(ann);

		return picat_unify(ret, picat_build_float(temp));
	}

	int pi_fann_set_learning_rate() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM rate = picat_get_call_arg(2, 2); DEREF(rate);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		float learning_rate = PICAT_GET_FLOAT(rate);

		fann_set_learning_rate(ann, learning_rate);

		return PICAT_TRUE;
	}

	int pi_fann_get_learning_momentum() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM ret = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		float temp = fann_get_learning_momentum(ann);

		return picat_unify(ret, picat_build_float(temp));
	}

	int pi_fann_set_learning_momentum() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM rate = picat_get_call_arg(2, 2); DEREF(rate);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		float learning_rate = PICAT_GET_FLOAT(rate);

		fann_set_learning_momentum(ann, learning_rate);

		return PICAT_TRUE;
	}


	int pi_get_activation_function() {
		TERM nn = picat_get_call_arg(1, 4);
		TERM l = picat_get_call_arg(2, 4);
		TERM n = picat_get_call_arg(3, 4);
		TERM ret = picat_get_call_arg(4, 4);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		int layer = picat_get_integer(l);
		int neuron = picat_get_integer(n);

		fann_activationfunc_enum func = fann_get_activation_function(ann, layer, neuron);

		switch (func) {
		case 0:
			return picat_unify(ret, picat_build_atom("FANN_LINEAR"));
		case 1:
			return picat_unify(ret, picat_build_atom("FANN_THRESHOLD"));
		case 2:
			return picat_unify(ret, picat_build_atom("FANN_THRESHOLD_SYMMETRIC"));
		case 3:
			return picat_unify(ret, picat_build_atom("FANN_SIGMOID"));
		case 4:
			return picat_unify(ret, picat_build_atom("FANN_SIGMOID_STEPWISE"));
		case 5:
			return picat_unify(ret, picat_build_atom("FANN_SIGMOID_SYMMETRIC"));
		case 6:
			return picat_unify(ret, picat_build_atom("FANN_SIGMOID_SYMMETRIC_STEPWISE"));
		case 7:
			return picat_unify(ret, picat_build_atom("FANN_GAUSSIAN"));
		case 8:
			return picat_unify(ret, picat_build_atom("FANN_GAUSSIAN_SYMMETRIC"));
		case 9:
			return picat_unify(ret, picat_build_atom("FANN_GAUSSIAN_STEPWISE"));
		case 10:
			return picat_unify(ret, picat_build_atom("FANN_ELLIOT"));
		case 11:
			return picat_unify(ret, picat_build_atom("FANN_ELLIOT_SYMMETRIC"));
		case 12:
			return picat_unify(ret, picat_build_atom("FANN_LINEAR_PIECE"));
		case 13:
			return picat_unify(ret, picat_build_atom("FANN_LINEAR_PIECE_SYMMETRIC"));
		case 14:
			return picat_unify(ret, picat_build_atom("FANN_SIGMOID_SYMMETRIC"));
		case 15:
			return picat_unify(ret, picat_build_atom("FANN_SIGMOID_SYMMETRIC_STEPWISE"));
		case 16:
			return picat_unify(ret, picat_build_atom("FANN_SIN_SYMMETRIC"));
		case 17:
			return picat_unify(ret, picat_build_atom("FANN_COS_SYMMETRIC"));
		case 18:
			return picat_unify(ret, picat_build_atom("FANN_SIN"));
		case 19:
			return picat_unify(ret, picat_build_atom("FANN_COS"));
		}
	}

	int pi_fann_set_activation_function() {
		TERM nn = picat_get_call_arg(1, 4);
		TERM act = picat_get_call_arg(2, 4);
		TERM l = picat_get_call_arg(3, 4);
		TERM n = picat_get_call_arg(4, 4);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		fann_activationfunc_enum activation_function = picat_to_fann_func(act);
		int layer = picat_get_integer(l);
		int neuron = picat_get_integer(n);

		fann_set_activation_function(ann, activation_function, layer, neuron);


		return PICAT_TRUE;
	}

	int pi_fann_set_activation_function_layer() {
		TERM nn = picat_get_call_arg(1, 3);
		TERM act = picat_get_call_arg(2, 3);
		TERM l = picat_get_call_arg(3, 3);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		fann_activationfunc_enum activation_function = picat_to_fann_func(act);
		int layer = picat_get_integer(l);

		fann_set_activation_function_layer(ann, activation_function, layer);


		return PICAT_TRUE;
	}

	int pi_fann_set_activation_function_hidden() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM act = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		fann_activationfunc_enum activation_function = picat_to_fann_func(act);

		fann_set_activation_function_hidden(ann, activation_function);

		return PICAT_TRUE;
	}

	int pi_fann_set_activation_function_output() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM act = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		fann_activationfunc_enum activation_function = picat_to_fann_func(act);

		fann_set_activation_function_output(ann, activation_function);

		return PICAT_TRUE;
	}

	int pi_fann_get_activation_steepness() {
		TERM nn = picat_get_call_arg(1, 4);
		TERM l = picat_get_call_arg(2, 4);
		TERM n = picat_get_call_arg(3, 4);
		TERM ret = picat_get_call_arg(4, 4);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		int layer = picat_get_integer(l);
		int neuron = picat_get_integer(n);

		fann_type temp = fann_get_activation_steepness(ann, layer, neuron);

		return picat_unify(ret, picat_build_float(temp));
	}

	int pi_fann_set_activation_steepness() {
		TERM nn = picat_get_call_arg(1, 4);
		TERM s = picat_get_call_arg(2, 4); DEREF(s);
		TERM l = picat_get_call_arg(3, 4);
		TERM n = picat_get_call_arg(4, 4);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];
		
	
		fann_type steepness = PICAT_GET_FLOAT(s);
		int layer = picat_get_integer(l);
		int neuron = picat_get_integer(n);

		fann_set_activation_steepness(ann, steepness, layer, neuron);

		return PICAT_TRUE;
	}

	int pi_fann_set_activation_steepness_layer() {
		TERM nn = picat_get_call_arg(1, 3);
		TERM s = picat_get_call_arg(2, 3); DEREF(s);
		TERM l = picat_get_call_arg(3, 3);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		fann_type steepness = PICAT_GET_FLOAT(s);
		int layer = picat_get_integer(l);

		fann_set_activation_steepness_layer(ann, steepness, layer);

		return PICAT_TRUE;
	}

	int pi_fann_set_activation_steepness_hidden() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM s = picat_get_call_arg(2, 2); DEREF(s);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		fann_type steepness = PICAT_GET_FLOAT(s);

		fann_set_activation_steepness_hidden(ann, steepness);

		return PICAT_TRUE;
	}

	int pi_fann_set_activation_steepness_output() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM s = picat_get_call_arg(2, 2); DEREF(s);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		fann_type steepness = PICAT_GET_FLOAT(s);

		fann_set_activation_steepness_output(ann, steepness);

		return PICAT_TRUE;
	}

	int pi_fann_get_train_error() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM ret = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		switch (fann_get_train_error_function(ann)) {
		case FANN_ERRORFUNC_LINEAR:
			return picat_unify(ret, picat_build_atom("FANN_ERRORFUNC_LINEAR"));
		case FANN_ERRORFUNC_TANH:
			return picat_unify(ret, picat_build_atom("FANN_ERRORFUNC_TANH"));
		}
	}

	int pi_fann_set_train_error_function() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM func = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		fann_errorfunc_enum train_error_function = FANN_ERRORFUNC_LINEAR;

		if (!strcmp(picat_get_atom_name(func), "FANN_ERRORFUNC_TAHN"))
			train_error_function = FANN_ERRORFUNC_TANH;

		fann_set_train_error_function(ann, train_error_function);

		return PICAT_TRUE;
	}

	int pi_fann_get_train_stop_function() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM ret = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		switch (fann_get_train_stop_function(ann)) {
		case FANN_STOPFUNC_MSE:
			return picat_unify(ret, picat_build_atom("FANN_STOPFUNC_MSE"));
		case FANN_STOPFUNC_BIT:
			return picat_unify(ret, picat_build_atom("FANN_STOPFUNC_BIT"));
		}
	}

	int pi_fann_set_train_stop_function() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM func = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		fann_stopfunc_enum train_stop_function = FANN_STOPFUNC_MSE;

		if (!strcmp(picat_get_atom_name(func), "FANN_STOPFUNC_BIT")) {
			train_stop_function = FANN_STOPFUNC_BIT;
		}

		fann_set_train_stop_function(ann, train_stop_function);

		return PICAT_TRUE;
	}

	int pi_fann_get_bit_fail_limit() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM ret = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		fann_type temp = fann_get_bit_fail_limit(ann);

		return picat_unify(ret, picat_build_float(temp));
	}

	int pi_fann_set_bit_fail_limit() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM limit = picat_get_call_arg(2, 2); DEREF(limit);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		fann_type bit_fail_limit = PICAT_GET_FLOAT(limit);

		fann_set_bit_fail_limit(ann, bit_fail_limit);

		return PICAT_TRUE;
	}

	//void fann_set_callback(ann, fann_callback_type callback) {};

	int pi_fann_get_quickprop_decay() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM ret = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		float temp = fann_get_quickprop_decay(ann);

		return picat_unify(ret, picat_build_float(temp));
	}

	int pi_fann_set_quickprop_decay() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM decay = picat_get_call_arg(2, 2); DEREF(decay);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		float quickprop_decay = PICAT_GET_FLOAT(decay);

		fann_set_quickprop_decay(ann, quickprop_decay);

		return PICAT_TRUE;
	}

	int pi_fann_get_quickprop_mu() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM ret = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		float temp = fann_get_quickprop_mu(ann);

		return picat_unify(ret, picat_build_float(temp));
	}

	int pi_fann_set_quickprop_mu() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM mu = picat_get_call_arg(2, 2); DEREF(mu);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		float quickprop_mu = PICAT_GET_FLOAT(mu);

		fann_set_quickprop_mu(ann, quickprop_mu);

		return PICAT_TRUE;
	}

	int pi_fann_get_rprop_increase_factor() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM ret = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		float temp = fann_get_rprop_increase_factor(ann);

		return picat_unify(ret, picat_build_float(temp));
	}

	int pi_fann_set_rprop_increase_factor() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM fact = picat_get_call_arg(2, 2); DEREF(fact);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		float rprop_increase_factor = PICAT_GET_FLOAT(fact);

		fann_set_rprop_increase_factor(ann, rprop_increase_factor);

		return PICAT_TRUE;
	}

	int pi_fann_get_rprop_decrease_factor() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM ret = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		float temp = fann_get_rprop_decrease_factor(ann);

		return picat_unify(ret, picat_build_float(temp));
	}

	int pi_fann_set_rprop_decrease_factor() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM fact = picat_get_call_arg(2, 2); DEREF(fact);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		float rprop_decrease_factor = PICAT_GET_FLOAT(fact);

		fann_set_rprop_decrease_factor(ann, rprop_decrease_factor);

		return PICAT_TRUE;
	}

	int pi_fann_get_rprop_delta_min() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM ret = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		float temp = fann_get_rprop_delta_min(ann);

		return picat_unify(ret, picat_build_float(temp));
	}

	int pi_fann_set_rprop_delta_min() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM delta = picat_get_call_arg(2, 2); DEREF(delta);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		float rprop_delta_min = PICAT_GET_FLOAT(delta);

		fann_set_rprop_delta_min(ann, rprop_delta_min);

		return PICAT_TRUE;
	}


	int pi_fann_get_rprop_delta_max() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM ret = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		float temp = fann_get_rprop_delta_max(ann);

		return picat_unify(ret, picat_build_float(temp));
	}

	int pi_fann_set_rprop_delta_max() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM delta = picat_get_call_arg(2, 2); DEREF(delta);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		float rprop_delta_max = PICAT_GET_FLOAT(delta);

		fann_set_rprop_delta_max(ann, rprop_delta_max);

		return PICAT_TRUE;
	}

	int pi_fann_get_rprop_delta_zero() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM ret = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		float temp = fann_get_rprop_delta_zero(ann);

		return picat_unify(ret, picat_build_float(temp));
	}

	int pi_fann_set_rprop_delta_zero() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM delta = picat_get_call_arg(2, 2); DEREF(delta);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		float rprop_delta_max = PICAT_GET_FLOAT(delta);

		fann_set_rprop_delta_zero(ann, rprop_delta_max);

		return PICAT_TRUE;
	}

	int pi_fann_get_sarprop_weight_decay_shift() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM ret = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		float temp = fann_get_sarprop_weight_decay_shift(ann);

		return picat_unify(ret, picat_build_float(temp));
	}

	int pi_fann_set_sarprop_weight_decay_shift() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM weight = picat_get_call_arg(2, 2); DEREF(weight);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		float sarprop_weight_decay_shift = PICAT_GET_FLOAT(weight);

		fann_set_sarprop_weight_decay_shift(ann, sarprop_weight_decay_shift);

		return PICAT_TRUE;
	}

	int pi_fann_get_sarprop_step_error_threshold_factor() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM ret = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		float temp = fann_get_sarprop_step_error_threshold_factor(ann);

		return picat_unify(ret, picat_build_float(temp));
	}

	int pi_fann_set_sarprop_step_error_threshold_factor() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM threshold = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		float sarprop_step_error_threshold_factor(threshold);

		fann_set_sarprop_step_error_threshold_factor(ann, sarprop_step_error_threshold_factor);

		return PICAT_TRUE;
	}

	int pi_fann_get_sarprop_step_error_shift() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM ret = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		float temp = fann_get_sarprop_step_error_shift(ann);

		return picat_unify(ret, picat_build_float(temp));
	}

	int pi_fann_set_sarprop_step_error_shift() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM s = picat_get_call_arg(2, 2); DEREF(s);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		float sarprop_step_error_shift = PICAT_GET_FLOAT(s);

		fann_set_sarprop_step_error_shift(ann, sarprop_step_error_shift);

		return PICAT_TRUE;
	}

	int pi_fann_get_sarprop_temperature() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM ret = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		float temp = fann_get_sarprop_temperature(ann);

		return picat_unify(ret, picat_build_float(temp));
	}


	int pi_fann_set_sarprop_temperature() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM s = picat_get_call_arg(2, 2); DEREF(s);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		float sarprop_temperature = PICAT_GET_FLOAT(s);

		fann_set_sarprop_temperature(ann, sarprop_temperature);

		return PICAT_TRUE;
	}

	int pi_fann_create_from_file() {
		TERM file = picat_get_call_arg(1, 2);
		TERM ret = picat_get_call_arg(2, 2);

		char* configuration_file = picat_string_to_cstring(file);


		anns[n_id] = fann_create_from_file(configuration_file);

		TERM temp = picat_build_structure((char*)"fann", 1);
		TERM id = picat_get_arg(1, temp);
		picat_unify(id, picat_build_integer(n_id));

		n_id++;

		return picat_unify(ret,temp);
	}

	int pi_fann_save() {
		TERM nn = picat_get_call_arg(1, 3);
		TERM file = picat_get_call_arg(2, 3);
		TERM ret = picat_get_call_arg(3, 3);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		char* configuration_file = picat_string_to_cstring(file);


		int temp = fann_save(ann, configuration_file);

		return picat_unify(ret, picat_build_integer(temp));
	}

	
	int pi_fann_save_to_fixed() {
		TERM nn = picat_get_call_arg(1, 2);
		TERM file = picat_get_call_arg(2, 2);

		TERM nid = picat_get_arg(1, nn);
		fann* ann = anns[picat_get_integer(nid)];

		char* configuration_file = picat_string_to_cstring(file);

		TERM ret = picat_get_call_arg(2, 2);

		int temp = fann_save_to_fixed(ann, configuration_file);

		return picat_unify(ret, picat_build_integer(temp));
	}
}

extern "C" int fann_cpreds() {
	insert_cpred((char*)"fann_destroy_all", 0, fann_destroy_all);
	insert_cpred((char*)"pi_get_data_in_out", 4, pi_get_data_in_out);
	insert_cpred((char*)"pi_fann_create_standard", 2, pi_fann_create_standard);
	insert_cpred((char*)"pi_fann_create_sparse", 3, pi_fann_create_sparse);
	insert_cpred((char*)"pi_fann_create_shortcut", 2, pi_fann_create_shortcut);
	insert_cpred((char*)"pi_fann_destroy", 1, pi_fann_destroy);
	insert_cpred((char*)"pi_fann_copy", 2, pi_fann_copy);
	insert_cpred((char*)"pi_fann_run", 3, pi_fann_run);
	insert_cpred((char*)"pi_fann_randomize_weights", 3, pi_fann_randomize_weights);
	insert_cpred((char*)"pi_fann_init_weights", 2, pi_fann_init_weights);
	insert_cpred((char*)"pi_fann_print_connections", 1, pi_fann_print_connections);
	insert_cpred((char*)"pi_fann_print_parameters", 1, pi_fann_print_parameters);
	insert_cpred((char*)"pi_fann_get_num_input", 2, pi_fann_get_num_input);
	insert_cpred((char*)"pi_fann_get_num_output", 2, pi_fann_get_num_output);
	insert_cpred((char*)"pi_fann_get_total_neurons", 2, pi_fann_get_total_neurons);
	insert_cpred((char*)"pi_fann_get_total_connections", 2, pi_fann_get_total_connections);
	insert_cpred((char*)"pi_fann_get_network_type", 2, pi_fann_get_network_type);
	insert_cpred((char*)"pi_fann_get_connection_rate", 2, pi_fann_get_connection_rate);
	insert_cpred((char*)"pi_fann_get_num_layers", 2, pi_fann_get_num_layers);
	insert_cpred((char*)"pi_fann_test", 4, pi_fann_test);
	insert_cpred((char*)"pi_fann_get_MSE", 2, pi_fann_get_MSE);
	insert_cpred((char*)"pi_fann_get_bit_fail", 2, pi_fann_get_bit_fail);
	insert_cpred((char*)"pi_fann_set_bit_fail_limit", 2, pi_fann_set_bit_fail_limit);
	insert_cpred((char*)"pi_fann_reset_MSE", 1, pi_fann_reset_MSE);
	insert_cpred((char*)"pi_fann_train_on_data", 5, pi_fann_train_on_data);
	insert_cpred((char*)"pi_fann_train_on_file", 5, pi_fann_train_on_file);
	insert_cpred((char*)"pi_fann_train_epoch", 3, pi_fann_train_epoch);
	insert_cpred((char*)"pi_fann_read_train_from_file", 2, pi_fann_read_train_from_file);
	insert_cpred((char*)"pi_fann_create_train", 4, pi_fann_create_train);
	insert_cpred((char*)"pi_fann_create_train_array", 6, pi_fann_create_train_array);
	insert_cpred((char*)"pi_fann_destroy_train", 1, pi_fann_destroy_train);
	insert_cpred((char*)"pi_fann_shuffle_train_data", 1, pi_fann_shuffle_train_data);
	insert_cpred((char*)"pi_fann_scale_train", 2, pi_fann_scale_train);
	insert_cpred((char*)"pi_fann_descale_train", 2, pi_fann_descale_train);
	insert_cpred((char*)"pi_fann_set_input_scaling_params", 5, pi_fann_set_input_scaling_params);
	insert_cpred((char*)"pi_fann_set_output_scaling_params", 5, pi_fann_set_input_scaling_params);
	insert_cpred((char*)"pi_fann_set_scaling_params", 7, pi_fann_set_scaling_params);
	insert_cpred((char*)"pi_fann_clear_scaling_params", 2, pi_fann_clear_scaling_params);
	insert_cpred((char*)"pi_fann_scale_input", 3, pi_fann_scale_input);
	insert_cpred((char*)"pi_fann_scale_output", 3, pi_fann_scale_output);
	insert_cpred((char*)"pi_fann_descale_input", 3, pi_fann_descale_input);
	insert_cpred((char*)"pi_fann_descale_output", 3, pi_fann_descale_output);
	insert_cpred((char*)"pi_fann_scale_input_train_data", 3, pi_fann_scale_input_train_data);
	insert_cpred((char*)"pi_fann_scale_output_train_data", 3, pi_fann_scale_output_train_data);
	insert_cpred((char*)"pi_fann_scale_train_data", 3, pi_fann_scale_train_data);
	insert_cpred((char*)"pi_fann_merge_train_data", 3, pi_fann_merge_train_data);
	insert_cpred((char*)"pi_fann_duplicate_train_data", 2, pi_fann_duplicate_train_data);
	insert_cpred((char*)"pi_fann_subset_train_data", 4, pi_fann_subset_train_data);
	insert_cpred((char*)"pi_fann_length_train_data", 2, pi_fann_length_train_data);
	insert_cpred((char*)"pi_fann_num_input_train_data", 2, pi_fann_num_input_train_data);
	insert_cpred((char*)"pi_fann_num_output_train_data", 2, pi_fann_num_output_train_data);
	insert_cpred((char*)"pi_fann_save_train", 3, pi_fann_save_train);
	insert_cpred((char*)"pi_fann_save_train_to_fixed", 4, pi_fann_save_train_to_fixed);
	insert_cpred((char*)"pi_fann_get_training_algorithm", 2, pi_fann_get_training_algorithm);
	insert_cpred((char*)"pi_fann_set_training_algorithm", 2, pi_fann_set_training_algorithm);
	insert_cpred((char*)"pi_fann_get_learning_rate", 2, pi_fann_get_learning_rate);
	insert_cpred((char*)"pi_fann_set_learning_rate", 2, pi_fann_set_learning_rate);
	insert_cpred((char*)"pi_fann_get_learning_momentum", 2, pi_fann_get_learning_momentum);
	insert_cpred((char*)"pi_fann_set_learning_momentum", 2, pi_fann_set_learning_momentum);
	insert_cpred((char*)"pi_get_activation_function", 4, pi_get_activation_function);
	insert_cpred((char*)"pi_fann_set_activation_function", 4, pi_fann_set_activation_function);
	insert_cpred((char*)"pi_fann_set_activation_function_layer", 3, pi_fann_set_activation_function_layer);
	insert_cpred((char*)"pi_fann_set_activation_function_hidden", 2, pi_fann_set_activation_function_hidden);
	insert_cpred((char*)"pi_fann_set_activation_function_output", 2, pi_fann_set_activation_function_output);
	insert_cpred((char*)"pi_fann_get_activation_steepness", 4, pi_fann_get_activation_steepness);
	insert_cpred((char*)"pi_fann_set_activation_steepness", 4, pi_fann_set_activation_steepness);
	insert_cpred((char*)"pi_fann_set_activation_steepness_layer", 3, pi_fann_set_activation_steepness_layer);
	insert_cpred((char*)"pi_fann_set_activation_steepness_hidden", 2, pi_fann_set_activation_steepness_hidden);
	insert_cpred((char*)"pi_fann_set_activation_steepness_output", 2, pi_fann_set_activation_steepness_output);
	insert_cpred((char*)"pi_fann_get_train_error", 2, pi_fann_get_train_error);
	insert_cpred((char*)"pi_fann_set_train_error_function", 2, pi_fann_set_train_error_function);
	insert_cpred((char*)"pi_fann_get_train_stop_function", 2, pi_fann_get_train_stop_function);
	insert_cpred((char*)"pi_fann_set_train_stop_function", 2, pi_fann_set_train_stop_function);
	insert_cpred((char*)"pi_fann_get_bit_fail_limit", 2, pi_fann_get_bit_fail_limit);
	insert_cpred((char*)"pi_fann_set_bit_fail_limit", 2, pi_fann_set_bit_fail_limit);
	insert_cpred((char*)"pi_fann_get_quickprop_decay", 2, pi_fann_get_quickprop_decay);
	insert_cpred((char*)"pi_fann_set_quickprop_decay", 2, pi_fann_set_quickprop_decay);
	insert_cpred((char*)"pi_fann_get_quickprop_mu", 2, pi_fann_get_quickprop_mu);
	insert_cpred((char*)"pi_fann_set_quickprop_mu", 2, pi_fann_set_quickprop_mu);
	insert_cpred((char*)"pi_fann_get_rprop_increase_factor", 2, pi_fann_get_rprop_increase_factor);
	insert_cpred((char*)"pi_fann_set_rprop_increase_factor", 2, pi_fann_set_rprop_increase_factor);
	insert_cpred((char*)"pi_fann_get_rprop_decrease_factor", 2, pi_fann_get_rprop_decrease_factor);
	insert_cpred((char*)"pi_fann_set_rprop_decrease_factor", 2, pi_fann_set_rprop_decrease_factor);
	insert_cpred((char*)"pi_fann_get_rprop_delta_min", 2, pi_fann_get_rprop_delta_min);
	insert_cpred((char*)"pi_fann_set_rprop_delta_min", 2, pi_fann_set_rprop_delta_min);
	insert_cpred((char*)"pi_fann_get_rprop_delta_max", 2, pi_fann_get_rprop_delta_max);
	insert_cpred((char*)"pi_fann_set_rprop_delta_max", 2, pi_fann_set_rprop_delta_max);
	insert_cpred((char*)"pi_fann_get_rprop_delta_zero", 2, pi_fann_get_rprop_delta_zero);
	insert_cpred((char*)"pi_fann_set_rprop_delta_zero", 2, pi_fann_set_rprop_delta_zero);
	insert_cpred((char*)"pi_fann_get_sarprop_weight_decay_shift", 2, pi_fann_get_sarprop_weight_decay_shift);
	insert_cpred((char*)"pi_fann_set_sarprop_weight_decay_shift", 2, pi_fann_set_sarprop_weight_decay_shift);
	insert_cpred((char*)"pi_fann_get_sarprop_step_error_threshold_factor", 2, pi_fann_get_sarprop_step_error_threshold_factor);
	insert_cpred((char*)"pi_fann_set_sarprop_step_error_threshold_factor", 2, pi_fann_set_sarprop_step_error_threshold_factor);
	insert_cpred((char*)"pi_fann_get_sarprop_step_error_shift", 2, pi_fann_get_sarprop_step_error_shift);
	insert_cpred((char*)"pi_fann_set_sarprop_step_error_shift", 2, pi_fann_set_sarprop_step_error_shift);
	insert_cpred((char*)"pi_fann_get_sarprop_temperature", 2, pi_fann_get_sarprop_temperature);
	insert_cpred((char*)"pi_fann_set_sarprop_temperature", 2, pi_fann_set_sarprop_temperature);
	insert_cpred((char*)"pi_fann_create_from_file", 2, pi_fann_create_from_file);
	insert_cpred((char*)"pi_fann_save", 3, pi_fann_save);
	insert_cpred((char*)"pi_fann_save_to_fixed", 2, pi_fann_save_to_fixed);



	return PICAT_TRUE;
}


#endif //endif FANN
