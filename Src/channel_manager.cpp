#include "channel_manager.h"

void
channel_manager::channel_manager( std::vector<user_equipment *> ,
								  base_station*,
								  params* )
{
	last_update_time = 0;
}

void
channel_manager::update( double t )
{
	update_channel_lsp( t );
	apply_spatial_consistancy( t );
	update_channel_ssp( t );
	update_channel_coefficients( t );

	last_update_time = t;
}

void 
channel_mananger::update_channel_lsp( t )
{

}

/*
void
channel_manager::apply_spatial_consistancy( t )
{
	double delta_t = t - last_update_time;

	//Find distance each UE has moved
	std::vector<double> ue_distances;
	for( user_equipment* ue : ues ) {
		double dx = ue->get_x_dot();
		double dy = ue->get_y_dot();
		ue_distances.push_back( t*sqrt( dx*dx + dy*dy ) );
	}

	//Find pair-wise distances
	arma::mat pairwise_distance (ues.size(), ues.size());
	for( int i = 0; i < ues.size(); i++ ) {
		for( int j = 0;  < ues.size(); j++ ) {
			pairwise_distance(i,j) = sqrt(
										pow(ues[i]->get_x() - ues[j]->get_x(),2)
									   +pow(ues[i]->get_y() - ues[j]->get_y(),2)
									     );
		}
	}
	
	//Determine correlation coefficients
	//Order: shadow, K, DS, ASA, ASD, <Old values>, <next ue>
	arma::mat covar (10*ues.size());
	covar.eye(); covar *= 0.5;

	for( int i = 0; i < ues.size() i++ ) {
		//Set correlation between old and new location
		int k = i*10;
		//Shadow has correlation distance of 10m
		covar(k, k+5) = exp( -1.0 * ue_distances[i] / 10.0 );
		//K factor: 4m
		covar(k+1, k+6) = exp( -1.0 * ue_distance[i] / 4.0 );
		//Delay spread: 6m
		covar(k+2, k+7) = exp( -1.0 * ue_distances[i] / 6.0 );
		//ASA: 5m
		covar(k+3, k+8) = exp( -1.0 * ue_distances[i] / 5.0 );
		//ASD: 7m
		covar(k+4, k+9) = exp( -1.0 * ue_distances[i] / 7.0 );

		for( int j = 0; j < ues.size() j++ ) {
			//Set correlation between users
			if( i >= j )
				continue;

			int l = j*10;
			covar(k, l)  
		}	
	}

	//Make symmetric
	covar = covar + covar.t();

	//Get values

	//Do Cholesky decomposition
	
	//Matrix multiply
	
	//Set values
}
*/
