struct params {
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
