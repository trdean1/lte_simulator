#include "channel.h"
#include "user_equipment.h"
#include "base_station.h"
#include "map.h"
#include "params.h"

#include <vector>
#include <stdio.h>

void print_parameters( FILE* stream, channel& c, bool header )
{
	if (header)
		fprintf( stream, "LoS, Distance, PL, Shadow, K, DS, ASA, ASD\n" );

	if( c.is_LoS() )
		fprintf( stream, "1, " );
	else
		fprintf( stream, "0, " );

	fprintf(stream, "%f, %f, %f, %f, %e, %f, %f\n", 
			c.get_distance(), c.get_pathloss(),
		    c.get_shadow(), c.get_K(),
		 	c.get_DS(), c.get_ASA(), c.get_ASD() );	
}

int main()
{
	//base_station bs (100, 100, 4, 0, 0.1);
	//user_equipment ue (30, 30, 0, 0);
	//int n_ue = 2;
	//map m (500, 500, n_ue, 1, 50, 5, 4, 0, 0.1);

	params sysp;
	sysp.f_c = 5e9;
	sysp.x_max =500;
	sysp.y_max = 500;
	sysp.is_static = false;
	sysp.is_tf_channel = true;
	sysp.n_users = 2;
	sysp.n_bs_antennas = 4;
	sysp.array_theta = 0;
	sysp.array_delta = 0.1;
	sysp.num_paths = 1;
	sysp.v_mu = 50;
	sysp.v_sigma = 5;
	sysp.v_cluster_sigma = 0;

	map m ( &sysp );

	base_station* bs = m.get_bs();
	std::vector<user_equipment*> ues = m.get_ue_list();
	std::vector<channel> channels;	
	for( int i = 0; i < sysp.n_users; i++ ) {
		channels.emplace_back( ues[i], bs, &sysp );
	}

	for( double t = 0; t < 1000.0; t += 100 ) {
		for( int i = 0; i < sysp.n_users; i++ ) {
			m.update_locations(100);
			channels[i].update_lsp_local(100);
			print_parameters( stdout, channels[i], i == 0 );
		}
		printf("---------------------------------------------\n");
	}
}
