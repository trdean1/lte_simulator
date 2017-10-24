#include "zak_channel.h"
#include "user_equipment.h"
#include "base_station.h"
#include "map.h"
#include "params.h"

#include <vector>
#include <stdio.h>

void print_zak_response( FILE* stream, zak_channel& c, int antenna_element ) 
{
		std::vector<zak_component> h = c.get_zak_response( antenna_element );

		int i;
		for( i = 0; i < h.size() - 1; i++ ) {
			fprintf( stream, "%f * e^(j * %f * t + %f) * delta( t - %e ) + ", h[i].amplitude, h[i].doppler, M_PI * h[i].phase / 180, h[i].delay );
		}
		fprintf( stream, "%f * e^(j * %f * t + %f) * delta( t - %e )\n", h[i].amplitude, h[i].doppler, M_PI * h[i].phase / 180, h[i].delay );
}


int main()
{
	//base_station bs (100, 100, 4, 0, 0.1);
	//user_equipment ue (30, 30, 0, 0);
	//int n_ue = 1;
	//int n_elements = 4;
	//map m (500, 500, n_ue, n_elements, 1, 10, 5, 0, 0.1);

	params sysp;
	sysp.f_c = 5e9;
	sysp.is_tf_channel = false;
	sysp.f_N = 3.2e7;
	sysp.N = 16;
	sysp.samp_per_symb = 8;
	sysp.impulse_width = 0.5;

	sysp.tau_resolution = 1e-7;
	sysp.nu_resolution = 10;
	sysp.delta_tau = 1e-6;
	sysp.delta_nu = 100;

	sysp.x_max = 500;
	sysp.y_max = 500;
	sysp.is_static = false;
	sysp.n_users = 1;
	sysp.n_bs_antennas = 4;
	sysp.array_theta = 0;
	sysp.array_delta = 0.1;
	sysp.num_paths = 1;
	sysp.v_mu = 10;
	sysp.v_sigma = 3;
	sysp.v_cluster_sigma = 0;
	map m ( &sysp );

	base_station* bs = m.get_bs();
	std::vector<user_equipment*> ues = m.get_ue_list();
	for( int i = 0; i < sysp.n_users; i++ ) {
		zak_channel c ( ues[i], bs, &sysp );
		c.update_lsp_local();
		c.update_ssp();
		c.compute_impulse_response( 0 );
		std::vector<zak_component> z = c.get_zak_response( 0 );
		for( int j = 0; j < sysp.n_bs_antennas; j++ ) {
		//	arma::cx_vec h = c.compute_coefficients( j );
			printf("Element %d\n", j);
			print_zak_response( stdout, c, j );
		}
		//print_delays( stdout, c, i == 0 );
		//print_powers( stdout, c, i == 0 );
		//print_aoa( stdout, c);
		//print_aod( stdout, c );
		//for( int j = 0; j < n_elements; j++ ) {
		//	printf("Element %d:\n", j);
		//	print_impulse_response( stdout, c, j );
		//}
	}

	printf("\n\n");
	for( int i = 0; i < sysp.n_users; i++ ) {
		zak_channel c ( ues[i], bs, &sysp );
		c.update_lsp_local();
		c.update_ssp();
		c.compute_impulse_response( 0 );
		for( int j = 0; j < sysp.n_bs_antennas; j++ ) {
			fast_alg::complex_num cj = c.get_coefficient( j, 1e-6, 100 );
			double isi = c.get_isi( j, 1e-6, 100 );
			double isi_2 = c.get_isi( j, 2e-6, 200 );
			printf("Element %d: %f + i %f (%f out of range, %f out of adj bins)\n", j, cj.real, cj.imag, isi, isi_2);
		}
	}
}
