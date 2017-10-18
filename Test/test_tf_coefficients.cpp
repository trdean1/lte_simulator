#include "tf_channel.h"
#include "user_equipment.h"
#include "base_station.h"
#include "map.h"
#include "params.h"

#include <vector>
#include <stdio.h>

void print_impulse_response( FILE* stream, tf_channel& c, int antenna_element ) 
{
		std::vector<impulse_pair> h = c.get_element_impulse( antenna_element );

		int i;
		for( i = 0; i < h.size() - 1; i++ ) {
			fprintf( stream, "( %f + i * %f ) * delta( t - %e ) + ", h[i].first.real, h[i].first.imag, h[i].second );
		}
		fprintf( stream, "( %f + i * %f ) * delta( t - %e )\n", h[i].first.real, h[i].first.imag, h[i].second );
}

void print_coefficients( FILE* stream, arma::cx_vec v ) 
{
	for( auto it : v ) {
		fprintf( stream, "%f + i * %f, ", it.real(), it.imag() ); 
	}

	fprintf( stream, "\n");
}

int main()
{
	//base_station bs (100, 100, 4, 0, 0.1);
	//user_equipment ue (30, 30, 0, 0);
	int n_ue = 1;
	int n_elements = 4;
	map m (500, 500, n_ue, n_elements, 0, 0.1);

	params sysp;
	sysp.f_c = 5e9;
	sysp.is_tf_channel = true;
	sysp.f_N = 3.2e7;
	sysp.N = 16;
	sysp.samp_per_symb = 8;
	sysp.impulse_width = 0.5;
	

	sysp.x_max = 200;
	sysp.y_max = 200;
	sysp.is_static = true;
	sysp.n_users = 1;
	sysp.n_bs_antennas = 4;
	sysp.array_theta = 0;
	sysp.array_delta = 0.1;
	sysp.num_paths = 1;
	sysp.v_mu = 0;
	sysp.v_sigma = 0;
	sysp.v_cluster_sigma = 0;

	base_station* bs = m.get_bs();
	std::vector<user_equipment*> ues = m.get_ue_list();
	for( int i = 0; i < n_ue; i++ ) {
		tf_channel c ( ues[i], bs, &sysp );
		c.update_lsp_local();
		c.update_ssp();
		c.compute_impulse_response( 0 );
		for( int j = 0; j < n_elements; j++ ) {
			arma::cx_vec h = c.compute_coefficients( j );
			printf("Element %d\n", j);
			print_coefficients( stdout, h );
		}
		//print_delays( stdout, c, i == 0 );
		//print_powers( stdout, c, i == 0 );
		//print_aoa( stdout, c);
		//print_aod( stdout, c );
		for( int j = 0; j < n_elements; j++ ) {
			printf("Element %d:\n", j);
			print_impulse_response( stdout, c, j );
		}
	}
}
