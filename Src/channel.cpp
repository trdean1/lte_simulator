#include "channel.h"

//Channel is reciprical but order matters because only one of these moves
//so we only bother rechecking the ue location each update
channel::channel( node* m_ue, node* m_bs, params* m_sysp ) 
{
	ue = m_ue;
	bs = m_bs;

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

void
channel::update() 
{
	distance = ue->get_distance( bs );
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
		mu = 1.06 - 0.1114 * log10( f_c_GHz );
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
