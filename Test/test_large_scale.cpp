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
	int n_ue = 500;
	map m (500, 500, n_ue, 4, 0, 0.1);

	params sysp;
	sysp.f_c = 5e9;
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

	base_station bs = m.get_bs();
	std::vector<user_equipment> ues = m.get_ue_list();
	for( int i = 0; i < n_ue; i++ ) {
		channel c ( &ues[i], &bs, &sysp );
		c.update();
		print_parameters( stdout, c, i == 0 );
	}
}
