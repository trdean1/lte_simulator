#pragma once

struct params {
	double f_c; //carrier frequency
	bool is_tf_channel;
	double f_N; //subcarrier spacing
	double N; //Number of subcarriers
	double samp_per_symb; //FFT size = samp_per_symb * N
	double impulse_width; //This is approximately the HWHM of an impulse in samples

	double nu_resolution;
	double tau_resolution;

	double x_max;
	double y_max;

	bool is_static;

	int n_users;
	int n_bs_antennas;

	double array_theta;	//Angle of array
	double array_delta; //Element spacing

	int num_paths;  //Paths UEs follow in mobile channel

	double v_mu;
	double v_sigma;

	double v_cluster_sigma;	//variance of speed of clusters
};
