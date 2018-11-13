#include "phy.h"

phy::phy( params* d_sysp )
{
	sysp = d_sysp;

	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	generator.seed( seed );
}

void
phy::generate_qams( arma::cx_cube* qams, int n, int m, int k, int order ) 
{
	qams->set_size( n, m, k );
	qams->imbue( [&]() { return generate_symbol( order ); } );
}

void
phy::convolve_qams( arma::cx_mat* qams, arma::cx_cube* output, channel* c )
{
	//Copy qams to output
	for( int k = 0; k < sysp->n_bs_antennas; k++ )
		output->slice(k) = *qams;

	if( sysp->is_tf_channel ) {
		tf_channel* tfc = static_cast<tf_channel*>(c);
		//Iterate over each antenna element
		for(int k = 0; k < sysp->n_bs_antennas; k++ ) {
			arma::cx_vec coeff = tfc->compute_coefficients( k );	
			//Iterate over each frequency bin
			for( int j = 0; j < qams->n_cols; j++ ) {
				//Apply gain, assume constant over each frequency bin
				for( int i = 0; i < qams->n_rows; i++ ) {
					output->at(i,j,k) *= coeff.at(j);
				}
			}
		}
	} else {
		zak_channel* zc = static_cast<zak_channel*>(c);
		//Iterate over each antenna element
		for( int k = 0; k < sysp->n_bs_antennas; k++ ) {
			fast_alg::complex_num coeff = 
				zc->get_coefficient( k, sysp->delta_tau, sysp->delta_nu );
			//multiply entire slice by coefficient
			std::complex<double> c = {coeff.real, coeff.imag};
			output->slice(k) *= c;
		}
	}
}

/* Note this only applies first order ISI, meaning we assume that symbols 
 * only interfere with the adjacent symbol
 */
void 
phy::add_isi( arma::cx_cube* output, channel* c )
{
	double isi;
	if( sysp->is_tf_channel ) {
		tf_channel* tfc = static_cast<tf_channel*>(c);

		for( int k = 0; k < output->n_slices; k++ ) {
			//Construct a circulant matrix where the first row is
			//[ 1 - isi   isi  0   0  ...  0 ]
			//except adjust the first and last rows to account for edge effects
			//Then the output is just this matrix time the qams
			isi = tfc->get_isi( k, sysp->cp_len );
			arma::vec ii = arma::zeros<arma::vec>( output->n_rows );
			ii.at(0) = 1 - isi;
			ii.at(1) = isi;
			arma::mat im = arma::circ_toeplitz( ii );
			im.at(0,0) = 1; im.at(0,1) = 0;
			im.at(output->n_rows - 1, 0) = 0;
			im = im.t();
			output->slice(k) = im * output->slice(k);
		}
	} else {
		zak_channel* zc = static_cast<zak_channel*>(c);
		//Iterate over each antenna element
		for( int k = 0; k < output->n_slices; k++ ) {
			isi = zc->get_isi( k, sysp->delta_tau, sysp->delta_nu );
			//No circulant trick here
			for( int i = 1; i < output->n_rows; i++ ) {
				for( int j = 1; j < output->n_cols; j++ ) {
					output->at(i,j,k) = (1 - isi ) * output->at(i,j,k) + 0.333 * isi *
						(output->at(i-1,j,k) + output->at(i,j-1,k) + output->at(i-1,j-1,k));
				}
			}
		}
	}
}

void
phy::add_awgn( arma::cx_cube* output, channel* c )
{
	double mod_power = average_power( sysp->modulation_order );
	double loss = c->get_total_loss();

	for( int k = 0; k < output->n_slices; k++ ) {
		if( sysp->is_tf_channel ) {
			tf_channel* tfc = static_cast<tf_channel*>(c);
			arma::cx_vec coeff = tfc->compute_coefficients( k );
			for( int i = 0; i < output->n_rows; i++ ) {
				double coeff_gain = 
					sqrt(coeff[i].real()*coeff[i].real() + coeff[i].imag()*coeff[i].imag());
				double rec_power = 
					mod_power * pow(10, (sysp->tx_pow - loss) / 10.0 ) * coeff_gain;
				double snr = rec_power / pow(10, sysp->rx_noise / 10.0 );
				double noise_power = coeff_gain * mod_power / snr;
				for( int j = 0; j < output->n_cols; j++ ) {
					output->at(i,j,k) += noise_power * randn(generator);
				}
			}		
		} else {
			zak_channel* zc = static_cast<zak_channel*>(c);
			fast_alg::complex_num coeff = 
				zc->get_coefficient( k, sysp->delta_tau, sysp->delta_nu );

			double coeff_gain = sqrt(coeff.real*coeff.real + coeff.imag*coeff.imag);
			double rec_power = mod_power * pow(10, (sysp->tx_pow - loss) / 10.0 ) * coeff_gain;
			double snr = rec_power / pow(10, sysp->rx_noise / 10.0 );
			double noise_power = coeff_gain * mod_power / snr;
			arma::mat re_noise = sqrt(noise_power) * 
				arma::randn( output->n_rows, output->n_cols );
			arma::mat im_noise = sqrt(noise_power) * 
				arma::randn( output->n_rows, output->n_cols );
			arma::cx_mat noise;
			noise.resize( output->n_rows, output->n_cols ); 
			noise.set_real( re_noise );
			noise.set_imag( im_noise );

			output->slice(k) += noise;
		}
	}
	
}

void
phy::qams_to_file( arma::cx_cube* qams, std::string filename )
{
	qams->save( filename, arma::raw_ascii );
}

double
phy::average_power( int order )
{
	double out = 0;
	for( int i = 1; i < 2*order - 1; i += 2 )
		out += i*i;

	return sqrt(out);
}

std::complex<double>
phy::generate_symbol( int order )
{
	std::uniform_int_distribution<double> randi(0, order - 1);
	std::complex<double> out = {randi(generator), randi(generator)};

	out *= 2;
	out -= std::complex<double>( order - 1, order - 1 );	

	return out;
}	
