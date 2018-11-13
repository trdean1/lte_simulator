#include "tf_channel_manager.h"

tf_channel_manager::tf_channel_manager( base_station* bs, 
										  std::vector<user_equipment*> ues,
										  params* d_sysp ) 
{
	sysp = d_sysp;
	for( user_equipment* ue : ues ) {
		tf_channel* z = new tf_channel( ue, bs, sysp );
		channels.push_back( z );
	}

	p = new phy( sysp );
}

void
tf_channel_manager::update_channels( double t )
{
	channel_manager::update_channels( t );
	for( channel* c : channels ) {
		tf_channel* tf = static_cast<tf_channel*>( c );
		tf->compute_impulse_response( t );
	}	
}

arma::cx_vec
tf_channel_manager::get_coefficients( int rx_index, int tx_index ) 
{
	tf_channel* c = static_cast<tf_channel*>( channels[rx_index] );
	return c->compute_coefficients( tx_index );
}

double
tf_channel_manager::first_order_isi( int rx_index, int tx_index )
{
	tf_channel* c = static_cast<tf_channel*>( channels[rx_index] );
	return c->get_isi( tx_index, sysp->cp_len );
}

double
tf_channel_manager::higher_order_isi( int rx_index, int tx_index )
{
	tf_channel* c = static_cast<tf_channel*>( channels[rx_index] );
	return c->get_isi( tx_index, 2*sysp->cp_len );
}

std::vector<impulse_pair> 
tf_channel_manager::get_full_impulse( int rx_index, int tx_index )
{
	tf_channel* c = static_cast<tf_channel*>( channels[rx_index] );
	return c->get_element_impulse( tx_index );
}
