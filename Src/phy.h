#include "channel.h"
#include "zak_channel.h"
#include "tf_channel.h"
#include "params.h"

#include <stdio.h>
#include <armadillo>
#include <random>
#include <chrono>
#include <string>

#pragma once

class phy {
	public:
		phy( params* d_sysp );

		void generate_qams( arma::cx_cube* qams, int n, int m, int k, int order );
		void convolve_qams( arma::cx_mat* qams, arma::cx_cube* output, channel* c);
		void add_isi(  arma::cx_cube* output, channel* c );
		void add_awgn( arma::cx_cube* output, channel* c);

		void qams_to_file( arma::cx_cube*, std::string filename  );

	private:
		std::complex<double> generate_symbol( int order );
		double average_power( int order );
		
		//channel* c;
		params* sysp;

		//dim 1 = time bins
		//dim 2 = freq bins
		//dim 3 = spatial bins
		//arma::cx_cube qams;
		//arma::cx_cube output;

		std::default_random_engine generator;
		std::normal_distribution<double> randn;
};
