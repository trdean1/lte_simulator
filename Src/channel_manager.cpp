#include "channel_manager.h"

channel_manager::channel_manager( )
{

}

channel_manager::~channel_manager()
{
	for( channel *c : channels ) 
		delete c;

	delete p;
}

void
channel_manager::update_channels( double t )
{
	for( channel* c : channels ) {
		c->update_lsp_local( t );
		c->update_ssp( );	
	}
}

double 
channel_manager::get_loss( int rx_index )
{
	return channels[rx_index]->get_total_loss();
}

void
channel_manager::generate_qams( )
{
	if( sysp->is_tf_channel ) {
		p->generate_qams( &qams, sysp->block_len, sysp->N, sysp->n_users, 
						  sysp->modulation_order );
	} else {
		p->generate_qams( &qams, 
			sqrt(sysp->N / sysp->zak_aspect), sqrt(sysp->N *  sysp->zak_aspect), 
			sysp->n_users, sysp->modulation_order );
	}
}

void
channel_manager::convolve_qams( )
{
	std::vector<arma::cx_cube> output_per_user (sysp->n_users);

	for( int i = 0; i < sysp->n_users; i++ ) {
		arma::cx_mat um = qams.slice(i);
		output_per_user[i].resize(qams.n_rows,qams.n_cols, sysp->n_bs_antennas);
		p->convolve_qams( &um, &output_per_user[i], channels[i] );
	}

	rx_signal = output_per_user[0];
	for( int i = 1; i < sysp->n_users; i++ )
		rx_signal += output_per_user[i];
}

void
channel_manager::output_qams( std::string filename )
{
	p->qams_to_file( &qams, filename );
}

void
channel_manager::output_output( std::string filename )
{
	p->qams_to_file( &rx_signal, filename );
}
