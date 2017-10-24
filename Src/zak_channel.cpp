#include "zak_channel.h"

/** Gives impulse response in the zak domain based on Winner II 
 * channel model.  Channel response is a complex constant times
 * and exponential giving doppler values and a delta function giving
 * delay values 
 *
 * t is ignored.  We include it here to maintain the interface
 * defined by channel
 *
 */
void
zak_channel::compute_impulse_response( double t )
{
	//Get location of antenna arrays
	base_station* tbs = static_cast<base_station*>( bs );
	std::vector<std::pair<double,double>> ba_locs = tbs->get_elements();

	//Get position and speed of UE
	double x = ue->get_x(); double y = ue->get_y();
	double vx = ue->get_x_dot(); double vy = ue->get_y_dot();

	std::pair<int,int> strongest = get_two_strongest_clusters();

	for( int i = 0; i < n_bs_elements; i++ ) {
		std::vector<zak_component> element_comp;

		for( int j = 0; j < clusters; j++ ) {

			//All rays have equal power
			double power = sqrt( cluster_powers[j] / rays ) ;

			std::vector<zak_component> cluster_comp (rays);
			/************** Compute Coefficients for each ray ********/
			for( int k = 0; k < rays; k++ ) {

				cluster_comp[k].amplitude = power;
				//Phase offset from multipath component
				cluster_comp[k].phase = 0;
				if( !LoS || j > 0 ) 
					cluster_comp[k].phase = phases[j][k];

				//Phase offset from rx array
				double dot = ba_locs[i].first  * fast_alg::cheb7_cos( aoa[j][k] ) 
						   + ba_locs[i].second * fast_alg::cheb7_sin( aoa[j][k] );

				//Phase offset from tx location
				dot += x * fast_alg::cheb7_cos( aod[j][k] ) 
					 + y * fast_alg::cheb7_sin( aod[j][k] );

				//Doppler from moving rx
			    double v = vx * fast_alg::cheb7_cos( aod[j][k] ) 
				     + vy * fast_alg::cheb7_sin( aod[j][k] );

				//Doppler from moving cluster
				v += cluster_velocity[j] * fast_alg::cheb7_cos( aod[j][k] ); 

				//Change distance to phase angle
				cluster_comp[k].phase *= 2 * M_PI * sysp->f_c / 3e8;
				cluster_comp[k].phase += dot;

				cluster_comp[k].doppler = v * sysp->f_c / 3e8;

				/************** Set delays **********************/
				//All LoS components have zero delay
				if( LoS && j == 0 ) {
					cluster_comp[k].delay = 0;
				} else if ( j != strongest.first && j != strongest.second ) {
					//All rays from the N-2 weakest clusters arrive at the same time.
					//We simply sum the powers coherently in this case.
					cluster_comp[k].delay = delays[j];
				} else {
					//For two strongest clusters, rays get lumped into three arrival
					//bins
					double delay_cluster_spread = 6.5622-3.4084*log10(sysp->f_c / 6.0);
					if( delay_cluster_spread < 0.25 )
						delay_cluster_spread = 0.25;

					delay_cluster_spread *= 10e-9;

					if( k <= 8 || k == 19 || k == 20 ) {
						cluster_comp[k].delay = delays[j];
					} else if ( k >= 13 && k <= 16 ) {
						cluster_comp[k].delay = delays[j] + 2.56 * delay_cluster_spread;
					} else {
						cluster_comp[k].delay = delays[j] + 1.28 * delay_cluster_spread;
					}
				}
			}


			/************* In the LoS case, we must rescale powers ***********/
			if (LoS) {
				double K_lin = pow(10.0, K / 10.0);
				for( int ii = 0; ii < cluster_comp.size(); ii++ ) {
					if( j == 0 ) {
						cluster_comp[ii].amplitude *= sqrt( rays * K_lin / (1.0 + K_lin) );
					} else {
						cluster_comp[ii].amplitude *= sqrt( 1.0 / (1.0 + K_lin) );
					}
				}
			}

			//Push into flat list of deltas per antenna element
			for( auto ci : cluster_comp )
				element_comp.push_back( ci );
			
		}
		element_comp = merge_ambiguous_components( element_comp, 
												   sysp->nu_resolution,
												   sysp->tau_resolution);

		zak_response_per_element.push_back( element_comp );
	}

}

/** Combines components that are close together in both delay and doppler domain
 * (considers \ell_\infty norm out of ease of implementation).  Close elements
 * are summed together coherently.  This is just an N^2 greedy algorithm
 */
std::vector<zak_component>
zak_channel::merge_ambiguous_components( std::vector<zak_component> all_Zs, 
										 double nu_res,
										 double tau_res )
{
	std::vector<zak_component> merged_Zs;

	//First step is to assign components to be grouped together.  This is stored
	//in the following data structure. -1 means it hasn't been assigned yet.
	std::unordered_map<zak_component, int, decltype(&hash_zak)> 
			group_assignment (0, hash_zak);

	//Set all to unassigned
	for( auto z : all_Zs )
		group_assignment[z] = -1;	

	int next_group = 0;
	for( auto z : all_Zs ) {
		if( group_assignment[z] != -1 )
			continue;

		//This component will be grouped this round, we need to now look for
		//matching components
		group_assignment[z] = next_group;
		bool found_assignment = false;

		//Check the \ell_\infty distance for all components
		for( auto z_i : all_Zs ) {
			if( group_assignment[z_i] != -1 )
				continue;

			if( ( fabs( z.delay - z_i.delay ) < tau_res ) &&
				( fabs( z.doppler - z_i.doppler ) < nu_res ) ) { 
				//Add to group
				group_assignment[z_i] = next_group;
				found_assignment = true;
			} 
		}



		//Find the elements in the newest cluster
		std::vector<zak_component> group;
		for( auto it = group_assignment.begin(); it != group_assignment.end(); it++ ) {
			if( it->second == next_group )
				group.push_back( it->first );
		}

		zak_component cluster = group[0];

		//If we found anything, we need to make a second pass
		if( found_assignment ) {
			for( auto z_i : group ) {
				for( auto z_j : all_Zs ) {
					if( group_assignment[z_j] != -1 || 
						group_assignment[z_j] == next_group )
						continue; 
	
					if( ( fabs( z_j.delay - z_i.delay ) < tau_res ) &&
						( fabs( z_j.doppler - z_i.doppler ) < nu_res ) ) { 
						//Add to group
						group_assignment[z_j] = next_group;
						group.push_back( z_j );
					} 
				}
			}	
		}

		//Merge cluster
		if ( group.size() > 1 ) {
			for( int i = 1; i < group.size(); i++ ) {
				//Sum coherently
				cluster.amplitude = sqrt( cluster.amplitude * cluster.amplitude +
										  group[i].amplitude * group[i].amplitude +
										  2 * group[i].amplitude * cluster.amplitude *
										  cos( group[i].phase - cluster.phase ) );
				cluster.phase += atan2( 
								group[i].amplitude * sin( group[i].phase - cluster.phase),
							    cluster.amplitude + 
								group[i].amplitude * cos( group[i].phase - cluster.phase) );	
			}
		} 

		merged_Zs.push_back(cluster);
		++next_group;
	}

	return merged_Zs;	
}
