#include "sysconfig.h"
#include "params.h"
#include "map.h"
#include "fast_alg.h"

#include <iostream>
#include <stdio.h>
#include <armadillo>

int main()
{
	FILE *fp = fopen("/Users/tom/IoT_Sim/conf/zak_static_example.conf", "r");
	if( fp == nullptr) {
		std::cerr << "Unable to open file" << std::endl;
		return 1;
	}
	sysconfig conf ( fp );
	params sysp = conf.param_from_file();

	map m ( &sysp );

	m.update( 10 );

	zak_channel_manager *z = static_cast<zak_channel_manager*>(m.get_cm());

	arma::cx_mat H( sysp.n_bs_antennas, sysp.n_users );

	printf("Channel Coefficients:\n");
	for( int i = 0; i < sysp.n_bs_antennas; i++ ) {
		for( int j = 0; j < sysp.n_users; j++ ) {
			fast_alg::complex_num c = z->get_coefficients(i,j);
			printf("(%f,%f), ", c.real, c.imag);

			H.at(i,j) = {c.real, c.imag};
		}
		printf("\n");
	}

	printf("\nCondition Number:\n");
	printf("%f\n", arma::cond(H));

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

	z->generate_qams();
	z->output_qams( "qams.dat" );

	z->convolve_qams();
	z->output_output( "out.dat" );
}
