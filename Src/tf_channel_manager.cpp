#include "tf_channel_manager.h"

tf_channel_manager::zak_channel_manager( base_station* bs, 
										  std::vector<user_equipment*> ues,
										  params* sysp ) 
{
	for( user_equipment* ue : ues ) {
		tf_channel* z = new tf_channel( bs, ue, sysp );
		channels.push_back( z );
	}
}

arma::cx_vec
tf_channel_manager::get_coefficients( int rx_index, int tx_index ) 
{
	return c[rx_index]->compute_coefficient( tx_index );
}

double
tf_channel_manager::first_order_isi( int rx_index, int tx_index )
{
	return c[rx_index]->get_isi( tx_index, sysp->cp_len );
}

double
tf_channel_manager::higher_order_isi( int rx_index, int tx_index )
{
	return c[rx_index]->get_isi( tx_index, 2*sysp->cp_len );
}

std::vector<impulse_pair> 
tf_channel_manager::get_full_impulse_response( int rx_index, int tx_index )
{
	return c[rx_index]->get_element_impulse( int tx_index );
}
