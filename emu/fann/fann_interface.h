#ifndef FANN_INTERFACE_H
#define FANN_INTERGACE_H

#ifdef FANN

#include "picat.h"
#include "floatfann.h"

#ifdef __cplusplus
extern "C" {
#endif

	int pi_fann_create_standard();
	int pi_fann_create_sparse();
	int pi_fann_create_shortcut();
	int pi_fann_destroy();
	//struct fann * fann_copy(struct fann *ann);
	int pi_fann_run();
	int pi_fann_randomize_weights();
	//void fann_init_weights(struct fann *ann, struct fann_train_data *train_data);
	int pi_fann_print_connections();
	int pi_fann_print_parameters();
	int pi_fann_get_num_input();
	int pi_fann_get_num_output();
	int pi_fann_get_total_neurons();
	int pi_fann_get_total_connections();
	int pi_fann_get_network_type();
	int pi_fann_get_connection_rate();
	int pi_fann_get_num_layers();
	/*
	void fann_get_layer_array(struct fann *ann, unsigned int *layers);

	void fann_get_bias_array(struct fann *ann, unsigned int *bias);

	void fann_get_connection_array(struct fann *ann, struct fann_connection *connections);


	void fann_set_weight_array(struct fann *ann, struct fann_connection *connections, unsigned int num_connections);

	void fann_set_weight(struct fann *ann, unsigned int from_neuron, unsigned int to_neuron, fann_type weight);

	void fann_set_user_data(struct fann *ann, void *user_data);

	void * fann_get_user_data(struct fann *ann);
	*/
	int pi_fann_train();
	int pi_fann_test();
	int pi_fann_get_MSE();
	int pi_fann_get_bit_fail();
	int pi_fann_reset_MSE();
	int pi_fann_train_on_data();
	int pi_fann_train_on_file();
	int pi_fann_train_epoch();
	// fann_test_data
	int pi_fann_read_train_from_file();
	int pi_fann_create_train();
	int pi_fann_destroy_train();
	int pi_fann_shuffle_train_data();
	int pi_fann_scale_train();
	int pi_fann_descale_train();
	int pi_fann_set_input_scaling_params();
	int pi_fann_set_output_scaling_params();
	int pi_fann_set_scaling_params();
	int pi_fann_clear_scaling_params();
	int pi_fann_scale_input();
	int pi_fann_scale_output();
	int pi_fann_descale_input();
	int pi_fann_descale_output();
	int pi_fann_scale_input_train_data();
	int pi_fann_scale_output_train_data();
	int pi_fann_scale_train_data();
	int pi_fann_merge_train_data();
	int pi_fann_duplicate_train_data();
	int pi_fann_subset_train_data();
	int pi_fann_length_train_data();
	int pi_fann_num_input_train_data();
	int pi_fann_num_output_train_data();
	int pi_fann_save_train();
	int pi_fann_save_train_to_fixed();
	int pi_fann_get_training_algorithm();
	int pi_fann_set_training_algorithm();
	int pi_fann_get_learning_rate();
	int pi_fann_set_learning_rate();
	int pi_fann_get_learning_momentum();
	int pi_fann_set_learning_momentum();
	int pi_get_activation_function();
	int pi_fann_set_activation_function();
	int pi_fann_set_activation_function_layer();
	int pi_fann_set_activation_function_hidden();
	int pi_fann_set_activation_function_output();
	int pi_fann_get_activation_steepness();
	int pi_fann_set_activation_steepness();
	int pi_fann_set_activation_steepness_layer();
	int pi_fann_set_activation_steepness_hidden();
	int pi_fann_set_activation_steepness_output();
	int pi_fann_get_train_error();
	int pi_fann_set_train_error_function();
	int pi_fann_get_train_stop_function();
	int pi_fann_set_train_stop_function();
	int pi_fann_set_bit_fail_limit();
	int pi_fann_get_quickdrop_decay();
	int pi_fann_set_quickdrop_decay();
	int pi_fann_get_quickprop_mu();
	int pi_fann_set_quickprop_mu();
	int pi_fann_get_rprop_increase_factor();
	int pi_fann_set_rprop_increase_factor();
	int pi_fann_get_rprop_decrease_factor();
	int pi_fann_set_rprop_decrease_factor();
	int pi_fann_get_rprop_delta_min();
	int pi_fann_set_rprop_delta_min();
	int pi_fann_get_rprop_delta_max();
	int pi_fann_set_rprop_delta_max();
	int pi_fann_get_rprop_delta_zero();
	int pi_fann_set_rprop_delta_zero();
	int pi_fann_get_sarprop_weight_decay_shift();
	int pi_fann_set_sarprop_weight_decay_shift();
	int pi_fann_get_sarprop_step_error_threshold_factor();
	int pi_fann_set_sarprop_step_error_threshold_factor();
	int pi_fann_get_sarprop_step_error_shift();
	int pi_fann_set_sarprop_step_error_shift();
	int pi_fann_get_sarprop_temperature();


#ifdef __cplusplus
}
#endif

#endif //endif FANN
#endif //FANN_INTERFACE_H