#include "params.h"
#include "sysconfig.h"
#include "map.h"
#include "fast_alg.h"

#include <iostream>
#include <stdio.h>
#include <armadillo>

int main()
{
	FILE *fp = fopen("/Users/tom/IoT_Sim/conf/tf_static_example.conf", "r");
	if( fp == nullptr) {
		std::cerr << "Unable to open file" << std::endl;
		return 1;
	}
	sysconfig conf ( fp );
	params sysp = conf.param_from_file();

	map m ( &sysp );

	m.update( 10 );

	tf_channel_manager *tf = static_cast<tf_channel_manager*>(m.get_cm());

	arma::cx_cube H( sysp.n_bs_antennas, sysp.n_users, sysp.N );

	printf("Channel Coefficients:\n");
	for( int i = 0; i < sysp.n_bs_antennas; i++ ) {
		for( int j = 0; j < sysp.n_users; j++ ) {
			arma::cx_vec hh = tf->get_coefficients(i,j);
			for( int k = 0; k < hh.size(); k++ ) {
				H.at(i,j,k) = hh[k];
			}

			std::cout << hh << std::endl;
		}
		printf("\n");
	}

	printf("\nCondition Numbers:\n");
	for( int i = 0; i < sysp.N; i++ )
		printf("%f\n", arma::cond(H.slice(i)));

	/*
	printf("\nISI:\n");
	for( int i = 0; i < sysp.n_bs_antennas; i++ ) {
		for( int j = 0; j < sysp.n_users; j++ ) {
			double isi = z->first_order_isi(i,j);
			printf("%f, ", isi);
		}
		printf("\n");
	}

	printf("\nPathloss + Shadowing:\n");
	for( int i = 0; i < sysp.n_users; i++ ) {
		printf("%f\n", z->get_loss( i ) );
	}
	*/
}
