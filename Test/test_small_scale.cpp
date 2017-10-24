#include "channel.h"
#include "user_equipment.h"
#include "base_station.h"
#include "map.h"
#include "params.h"

#include <vector>
#include <stdio.h>

void print_delays( FILE* stream, channel& c, bool header )
{
	if( header )
		fprintf( stream, "LoS, K, delay" );

	if( c.is_LoS() )
		fprintf( stream, "1, " );
	else
		fprintf( stream, "0, " );

	std::vector<double> delays = c.get_delays();
	double K = c.get_K();

	fprintf( stream, "%f, ", K );
	for( auto d : delays )
		fprintf( stream, "%e, ", d );

	fprintf( stream, "\n");
}

void print_powers( FILE* stream, channel& c, bool header )
{
	if( header )
		fprintf( stream, "LoS, K, power" );

	if( c.is_LoS() )
		fprintf( stream, "1, " );
	else
		fprintf( stream, "0, " );

	std::vector<double> pows = c.get_cluster_powers();

	for( auto d : pows )
		fprintf( stream, "%f, ", d );

	fprintf( stream, "\n");
}

void print_aoa( FILE* stream, channel& c ) 
{
	fprintf( stream, "Distance: %f, LoS AOA: %f, LoS AOD: %f, ASA: %f, ASD: %f\n", c.get_distance(), 
					  c.get_los_aoa(),  c.get_los_aod(),
		  		      c.get_ASA(), c.get_ASD() );
	
	std::vector<std::vector<double>> aoa = c.get_aoa();
	for( auto v : aoa ) {
		for( auto d : v ) {
			fprintf(stream, "%f, ", d);
		}
		fprintf(stream, "\n");
	}		
	fprintf(stream,"\n");
}

void print_aod( FILE* stream, channel& c ) 
{
	fprintf( stream, "Distance: %f, LoS AOA: %f, LoS AOD: %f, ASA: %f, ASD: %f\n", c.get_distance(), 
					  c.get_los_aoa(),  c.get_los_aod(),
		  		      c.get_ASA(), c.get_ASD() );
	
	std::vector<std::vector<double>> aod = c.get_aod();
	for( auto v : aod ) {
		for( auto d : v ) {
			fprintf(stream, "%f, ", d);
		}
		fprintf(stream, "\n");
	}		
	fprintf(stream,"\n");
}

int main()
{
	//base_station bs (100, 100, 4, 0, 0.1);
	//user_equipment ue (30, 30, 0, 0);
	//int n_ue = 50;
	//map m (500, 500, n_ue, 4, 0, 0.1);

	params sysp;
	sysp.f_c = 5e9;
	sysp.x_max = 500;
	sysp.y_max = 500;
	sysp.is_static = true;
	sysp.is_tf_channel = true;
	sysp.n_users = 50;
	sysp.n_bs_antennas = 4;
	sysp.array_theta = 0;
	sysp.array_delta = 0.1;
	sysp.num_paths = 1;
	sysp.v_mu = 0;
	sysp.v_sigma = 0;
	sysp.v_cluster_sigma = 0;

	map m ( &sysp );

	base_station* bs = m.get_bs();
	std::vector<user_equipment*> ues = m.get_ue_list();
	for( int i = 0; i < sysp.n_users; i++ ) {
		channel c ( ues[i], bs, &sysp );
		c.update_lsp_local();
		c.update_ssp();
		//print_delays( stdout, c, i == 0 );
		//print_powers( stdout, c, i == 0 );
		//print_aoa( stdout, c);
		print_aod( stdout, c );
	}
}
