#include "tf_channel.h"
		
void
tf_channel::compute_impulse_response( double t )
{
	//Get location of antenna arrays
	base_station* tbs = static_cast<base_station*>( bs );
	std::vector<std::pair<double,double>> ba_locs = tbs->get_elements();

	//Get position and speed of UE
	double x = ue->get_x(); double y = ue->get_y();
	double vx = ue->get_x_dot(); double vy = ue->get_y_dot();

	std::pair<int,int> strongest = get_two_strongest_clusters();

	for( int i = 0; i < n_bs_elements; i++ ) {

		std::vector<impulse_pair> impulses;
		for( int j = 0; j < clusters; j++ ) {

			//All rays have equal power
			double power = sqrt( cluster_powers[j] / rays ) ;

			/************** Compute Coefficients for each ray ********/
			std::vector<fast_alg::complex_num> ray_coeffs;
			for( int k = 0; k < rays; k++ ) {
				//Phase offset from multipath component
				double theta = 0;
				if( !LoS || j > 0 ) 
					theta = phases[j][k];

				//Phase offset from rx array
				double dot = ba_locs[i].first  * fast_alg::cheb7_cos( aoa[j][k] ) 
						   + ba_locs[i].second * fast_alg::cheb7_sin( aoa[j][k] );

				//Phase offset from tx location
				dot += x * fast_alg::cheb7_cos( aod[j][k] ) 
					 + y * fast_alg::cheb7_sin( aod[j][k] );

				//Phase offset from moving rx
			    dot += t * vx * fast_alg::cheb7_cos( aod[j][k] ) 
				     + t * vy * fast_alg::cheb7_sin( aod[j][k] );

				//Phase offset from moving cluster
				dot += t * cluster_velocity[j] * fast_alg::cheb7_cos( aod[j][k] ); 

				//Change distance to phase angle
				dot *= 2 * M_PI * sysp->f_c / 3e8;
				theta += dot;

				//Change phase angle to complex number
				fast_alg::complex_num phase = fast_alg::cheb7_exp( theta );

				//Apply phase angle to power
				ray_coeffs.push_back( {power * phase.real, power * phase.imag} ); 
			}

			/*********** Compute impulse response for each cluster **********/
			std::vector<impulse_pair> cluster_impulse; 

			//LoS component arrives with zero delay		
			if( LoS && j == 0 ) {
				cluster_impulse.push_back( {ray_coeffs[0], 0} );
			}

			//All rays from the N-2 weakest clusters arrive at the same time.
			//We simply sum the powers coherently in this case.
			else if( j != strongest.first && j != strongest.second ) {
					fast_alg::complex_num cluster_coeff = ray_coeffs[0];
					for( int k = 1; k < rays; k++ ) {
						cluster_coeff.real += ray_coeffs[k].real;
						cluster_coeff.imag += ray_coeffs[k].imag;
					}
					cluster_impulse.push_back( {cluster_coeff, delays[j]} );
			} else {
				//For two strongest clusters, rays get lumped into three arrival
				//bins
				double delay_cluster_spread = 6.5622-3.4084*log10(sysp->f_c / 6.0);
				if( delay_cluster_spread < 0.25 )
					delay_cluster_spread = 0.25;

				delay_cluster_spread *= 10e-9;

				cluster_impulse.push_back( { {0,0}, delays[j]} );
				cluster_impulse.push_back( { {0,0}, delays[j] + 1.28 * delay_cluster_spread} );
				cluster_impulse.push_back( { {0,0}, delays[j] + 2.56 * delay_cluster_spread} );

				//Sum coherently within each bin, ray assigment given by
				//standard
				for( int k = 0; k < rays; k++ ) {
					if( k <= 8 || k == 19 || k == 20 ) {
						cluster_impulse[0].first.real += ray_coeffs[k].real;
						cluster_impulse[0].first.imag += ray_coeffs[k].imag;
					} else if( k >= 13 && k <= 16 ) {
						cluster_impulse[2].first.real += ray_coeffs[k].real;
						cluster_impulse[2].first.imag += ray_coeffs[k].imag;
					} else {
						cluster_impulse[1].first.real += ray_coeffs[k].real;
						cluster_impulse[1].first.imag += ray_coeffs[k].imag;
					}
				}
			}
			/************* In the LoS case, we must rescale powers ***********/
			if (LoS) {
				double K_lin = pow(10.0, K / 10.0);
				for( int ii = 0; ii < cluster_impulse.size(); ii++ ) {
					if( j == 0 ) {
						cluster_impulse[ii].first.real *= sqrt( rays * K_lin / (1.0 + K_lin) );
						cluster_impulse[ii].first.imag *= sqrt( rays * K / (1.0 + K_lin) );
					} else {
						cluster_impulse[ii].first.real *= sqrt( 1.0 / (1.0 + K_lin) );
						cluster_impulse[ii].first.imag *= sqrt( 1.0 / (1.0 + K_lin) );
					}
				}
			}

			//Push into flat list of deltas per antenna element
			for( auto ci : cluster_impulse )
				impulses.push_back( ci );
			
		}
		//Sort so delay zero is first
		std::sort( impulses.begin(), impulses.end(), impulse_comp );
		impulse_response_per_element.push_back( impulses );
	}
}

bool
tf_channel::impulse_comp( impulse_pair a, impulse_pair b ) 
{
	return a.second < b.second;
}

/** Given channel impulse response, this function
 * computes the gain coefficients per OFDM frequency bin
 *
 * First, we must sample the impulse response function, then
 * we will take an FFT of this function and finally we will
 * coherently sum over each OFDM frequency bin
 */
arma::cx_vec 
tf_channel::compute_coefficients( int element_index )
{
	double T_s = 1 / sysp->samp_per_symb;
	T_s /= sysp->f_N;
	int FFT_size = sysp->N * sysp->samp_per_symb;

	//Step 1: find impulse response in sampled domain
	arma::cx_vec h = sample_impulse_response( element_index,
											  T_s,
											  sysp->impulse_width,
											  FFT_size );

	//Normalize
	h = arma::normalise(h);

	//Step 2: take fft
	arma::cx_vec H = arma::fft( h );

	//Step 3: coherently sum within each bin
	arma::cx_vec output( sysp->N );
	output.zeros();

	for( int i = 0; i < FFT_size; i += sysp->samp_per_symb ) {
		for( int j = 0; j < sysp->samp_per_symb; j++ ) {
			output[ i / sysp->samp_per_symb ] += H[i + j];
		}
	}

	//Normalize again
	output = arma::normalise( output );

	return output;
}

/** Returns a length N vector that gives that channel impulse response
 * sampled of the channel at element_index at rate T_s, assuming that 
 * each delta function is actually a Gaussian with a half-width at half 
 * max of impulse_width samples.  
 *
 */
arma::cx_vec
tf_channel::sample_impulse_response( int element_index, double T_s, 
				                     double impulse_width, double N )
{
	arma::cx_vec output(N);
	output.zeros();

	std::vector<impulse_pair> impulse_response = 
			impulse_response_per_element[element_index];

	//Iterate over each arriving ray	
	for( int i = 0; i < impulse_response.size(); i++ ) {
		double tau = impulse_response[i].second;
		arma::cx_double p = 
			{impulse_response[i].first.real, impulse_response[i].first.imag};

		//Each impulse is a very narrow Gaussian pulse
		for( int j = 0; j < N; j++ ) {
			output[j] += p*exp( -1.0 * pow(( j*T_s - tau ), 2.0) 
							  / pow(impulse_width*T_s, 2.0) );
		}
	}

	return output;
}

/** This simply determines the amount of power that lies outside of one cp
 * length.  This is the amount of ISI that will remain after CP removal.  We can
 * approximate this as a first order effect -- i.e. that all the power just
 * spills over into the next symbol.  A perfectly reasonable model since each 
 * symbol is drawn i.i.d.
 *
 */
double 
tf_channel::get_isi( int element_index, double cp_len )
{

	double oor_power = 0;

	for( impulse_pair p : impulse_response_per_element[element_index] ) {
		if( p.second > cp_len ) 
			oor_power += sqrt( p.first.real*p.first.real + p.first.imag*p.first.imag );
		
	}

	return oor_power;
}
