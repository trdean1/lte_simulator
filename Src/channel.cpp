#include "channel.h"

//Channel is reciprical but order matters because only one of these moves
//so we only bother rechecking the ue location each update
channel::channel( node* m_ue, node* m_bs, params* m_sysp ) 
{
	ue = m_ue;
	bs = m_bs;

	base_station* tbs = static_cast<base_station*>( bs );
	n_bs_elements = tbs->n_elements();

	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	generator.seed( seed );
	distance = ue->get_distance( bs );

	determine_LoS();

	last_shadow = NAN;
	last_K = NAN;
	last_DS = NAN;
	last_ASA = NAN;
	last_ASD = NAN;

	sysp = m_sysp;

	rays = 20;
	clusters = LoS ? 12 : 20;
}

//Generates large-scale parameters.  These parameters have the proper
//cross-correlations but are not spatially consistent
void
channel::update_lsp_local() 
{
	distance = ue->get_distance( bs );
	los_aoa = degtorad * ue->get_los_aoa( bs );
	los_aod = degtorad * ue->get_los_aod( bs );
	determine_LoS();
	clusters = LoS ? 12 : 20;
	
	cross_correlate();

	compute_pathloss();
	compute_shadowfading();
	compute_K();
	compute_DS();
	compute_ASD();
	compute_ASA();
}

void
channel::update_ssp()
{
	compute_delays();
	compute_cluster_powers();
	compute_aod();
	compute_aoa();
	generate_phases();
	compute_cluster_doppler();
}

//Determine whether or not the link is LoS based on Table 7.4.2-1
void
channel::determine_LoS()
{
	if( distance < 18.0 )
		LoS = true;
	else {
		double pr = (18.0 / distance) + 
				    exp( -1.0 * distance / 63.0 ) * 
					( 1.0 - 18.0 / distance );

		double y = randu( generator );
		if( y < pr )
			LoS = true;
		else
			LoS = false;
	}
}

void
channel::compute_pathloss()
{
	double d_critical = 4*1.5*10*sysp->f_c / 3e8;

	double f_c_GHz = sysp->f_c / 1e9;

	//Zero pathloss if within one meter
	if( distance < 1.0 ) {
		pathloss = 0;
		return;
	//Out of range
	} else if( distance > 5000 ) {
		pathloss = 150;
		return;
	//Within critical distance (two-ray model thing)
	} else if( distance < d_critical ) {
		pathloss = 28.0 + 22.0 * log10( distance ) + 20.0 * log10( f_c_GHz );
	//Outside critical distance
	} else {
		pathloss = 28.0 + 40.0 * log10( distance ) + 20.0 * log10( f_c_GHz ) 
				 - 9.0 * log10( d_critical*d_critical + 70 );
	}

	if( !LoS ) {
		double p_alt = 13.54 + 39.08 * log10( distance ) 
				     + 20.0 * log10( f_c_GHz ) - 0.6*8.5;
		if( p_alt > pathloss )
			pathloss = p_alt;
	}
}

void
channel::compute_shadowfading()
{
	last_shadow = shadow_dB;

	double sigma = LoS ? 4.0 : 6.0;
	shadow_dB = sigma * sfn;
}

void
channel::compute_K()
{
	last_K = K;
	K = 9.0 + 3.5 * kn;
}

void
channel::compute_DS()
{
	double mu;
	double sigma;

	double f_c_GHz = sysp->f_c / 1e9;
	if ( f_c_GHz < 6.0 )
		f_c_GHz = 6.0;

	if( LoS ) {
		mu = -6.955 - 0.0963 * log10( f_c_GHz );
		sigma = 0.66; 
	} else {
		mu = -6.28 - 0.204 * log10( f_c_GHz );
		sigma = 0.39;
	}

	double ds_log = mu + sigma * dsn;
	
	last_DS = DS;
	DS = pow( 10.0, ds_log );
}

void
channel::compute_ASD()
{
	double mu;
	double sigma;

	double f_c_GHz = sysp->f_c / 1e9;
	if ( f_c_GHz < 6.0 )
		f_c_GHz = 6.0;

	if( LoS ) {
		mu = 1.06 + 0.1114 * log10( f_c_GHz );
		sigma = 0.28; 
	} else {
		mu = 1.5 - 0.1144 * log10( f_c_GHz );
		sigma = 0.28;
	}

	double asd_log = mu + sigma * asdn;
	
	last_ASD = ASD;
	ASD = pow( 10.0, asd_log );

	if ( ASD > 104.0 ) 
		ASD = 104.0;
}


void
channel::compute_ASA()
{
	double mu;
	double sigma;

	double f_c_GHz = sysp->f_c / 1e9;
	if ( f_c_GHz < 6.0 )
		f_c_GHz = 6.0;

	if( LoS ) {
		mu = 1.81;
		sigma = 0.2; 
	} else {
		mu = 2.08 - 0.27 * log10( f_c_GHz );
		sigma = 0.11;
	}

	double asa_log = mu + sigma * asan;
	
	last_ASA = ASA;
	ASA = pow( 10.0, asa_log );

	if( ASA > 104.0 )
		ASA = 104.0;
}

//This is a matrix multiplication based on the cholesky factor of the 
//cross correlations listed in the standard
void
channel::cross_correlate()
{
	kn = randn( generator );
	dsn = randn( generator );
	asan = randn( generator );
	asdn = randn( generator );
	sfn = randn( generator );

	if( LoS ) {
   		dsn = 0.4000*asdn+0.8000*asan+0.4472*dsn;
   		sfn = -0.5000*asdn-0.5000*asan+0.4472*dsn+0.5477*sfn;
        kn = -0.2000*asan-0.5367*dsn+0.2556*sfn+0.7789*kn;
	} else {
    	asan = 0.4000*asdn + 0.9165*asan;
    	dsn = 0.4000*asdn + 0.4801*asan + 0.7807*dsn;
    	sfn = -0.5000*asdn + 0.2182*asan - 0.3904*dsn + 0.7416*sfn;
	}
}

void
channel::compute_delays()
{
	delays.clear();

	double r_tau = LoS ? 2.3 : 2.5;
	for( int i = 0; i < clusters; i++ ) {
		delays.push_back( -1.0 * r_tau * DS * log( randu(generator) ) );
	}

	std::sort( delays.begin(), delays.end() );

	//Normalize so smallest delay is zero
	for( int i = clusters - 1; i >= 0; i-- ) {
		delays[i] -= delays[0];
	}

	//Generate scaled delays for LoS case
	if( LoS ) {
		double c_tau = 0.7705 - 0.0433*K + 0.0002*K*K + 0.000017*K*K*K;
		scaled_delays = delays;
		for( int i = 0; i < clusters; i++ ) {
			scaled_delays[i] /= c_tau;
		}
	}
}

void
channel::compute_cluster_powers()
{
	cluster_powers.clear();

	double r_tau = LoS ? 2.3 : 2.5;

	double sum = 0;
	for( int i = 0; i < clusters; i++ ) {
		double Z_n = 3.0 * randn( generator );
		double power =  
			exp( -1.0 * delays[i] * ( r_tau - 1 ) / ( r_tau * DS ) )
			* pow( 10.0, Z_n / -10.0 );
		sum += power;
		cluster_powers.push_back( power );
	}

	//Normalize to one
	for( int i = 0; i < clusters; i++ ) 
		cluster_powers[i] /= sum;

	max_power = *std::max_element( cluster_powers.begin(), cluster_powers.end() );

	//Largest cluster gets rescaled for LoS case, but this is only used 
	//in angle generation
	if( LoS ) {
		double new_sum = sum;
	   	new_sum -= cluster_powers[0];
		scaled_cluster_powers.clear();
		scaled_cluster_powers.push_back( K / (K + 1) );

		for( int i = 1; i < clusters; i++ ) {
			scaled_cluster_powers.push_back( 
				(1.0 / (K + 1)) * cluster_powers[i] * sum / new_sum );
		}

		max_power = scaled_cluster_powers[0];
	}


}

void
channel::compute_aod()
{
	double c_phi;
	double phi_tick;
	double c_asd;

	aod.clear();

	std::vector<double> cluster_aod;

	for( int i = 0; i < clusters; i++ ) {
		int X_n = 2*round( randu( generator ) ) - 1;
		double Y_n = ASD * randn( generator ) / 7.0;

		if( LoS ) {
			c_asd = 5;
			c_phi = 1.146 * ( 1.1035 - 0.028 * K - 0.002 * K * K + 0.0001 * K * K * K );
			phi_tick = 2 * ASD * sqrt( -1.0 * log( scaled_cluster_powers[i] / max_power ) )  
					 / ( 1.4 * c_phi );

			if( i == 0 )
				cluster_aod.push_back( los_aod );
			else
				cluster_aod.push_back( X_n * phi_tick + Y_n + los_aod );

		} else {
			c_asd = 2;
			c_phi = 1.289;
			phi_tick = 2 * ASD * sqrt( -1.0 * log( cluster_powers[i] / max_power ) )  
					 / ( 1.4 * c_phi );

			cluster_aod.push_back( X_n * phi_tick + Y_n + los_aod );
		}

		std::vector<double> ray_aod;
		for( int j = 0; j < rays; j++ ) {
			ray_aod.push_back( cluster_aod[i] + c_asd * ray_basis[j] );
		}

		aod.push_back( ray_aod );
	}
}

void
channel::compute_aoa()
{
	double c_phi;
	double phi_tick;
	double c_asa;

	aoa.clear();

	std::vector<double> cluster_aoa;

	for( int i = 0; i < clusters; i++ ) {
		int X_n = 2*round( randu( generator ) ) - 1;
		double Y_n = ASD * randn( generator ) / 7.0;
		if( LoS ) {
			c_asa = 5;
			c_phi = 1.146 * ( 1.1035 - 0.028 * K - 0.002 * K * K + 0.0001 * K * K * K );
			phi_tick = 2 * ASA * sqrt( -1.0 * log( scaled_cluster_powers[i] / max_power ) )  
					 / ( 1.4 * c_phi );

			if( i == 0 )
				cluster_aoa.push_back( los_aoa );
			else
				cluster_aoa.push_back( X_n * phi_tick + Y_n + los_aoa );

		} else {
			c_asa = 2;
			c_phi = 1.289;
			phi_tick = 2 * ASA * sqrt( -1.0 * log( cluster_powers[i] / max_power ) )  
					 / ( 1.4 * c_phi );

			cluster_aoa.push_back( X_n * phi_tick + Y_n + los_aoa );
		}

		std::vector<double> ray_aoa;
		for( int j = 0; j < rays; j++ ) {
			ray_aoa.push_back( cluster_aoa[i] + c_asa * ray_basis[j] );
		}

		aoa.push_back( ray_aoa );
	}
}

void
channel::generate_phases()
{
	phases.clear();

	for( int i = 0; i < clusters; i++ ) {
		std::vector<double> tmp;
		for( int j = 0; j < rays; j++ ) {
			tmp.push_back( 2*M_PI*randu( generator ) );
		}
		phases.push_back( tmp );
	}
}

void 
channel::compute_cluster_doppler()
{
	for( int i = 0; i < clusters; i++ ) {
		cluster_velocity.push_back( sysp->v_cluster_sigma * randn( generator ) ); 
	}
}

std::pair<int,int>
channel::get_two_strongest_clusters()
{
	std::vector<double> cp_copy = cluster_powers;
	std::sort( cp_copy.begin(), cp_copy.end() );

	std::pair<int,int> result;
	for( int i = 0; i < cluster_powers.size(); i++ ) {
		if( cluster_powers[i] == cp_copy[0] )
			result.first = i;
		if( cluster_powers[i] == cp_copy[1] )
			result.second = i;
	}

	return result;
}	
