#include "zak_channel_manager.h"

zak_channel_manager::zak_channel_manager( base_station* bs, 
										  std::vector<user_equipment*> ues,
										  params* d_sysp ) 
{
	sysp = d_sysp;
	for( user_equipment* ue : ues ) {
		zak_channel* z = new zak_channel( ue, bs, sysp );
		channels.push_back( z );
	}

	p = new phy( sysp );
}

void
zak_channel_manager::update_channels( double t )
{
	channel_manager::update_channels( t );
	for( channel* c : channels ) {
		zak_channel* z = static_cast<zak_channel*>(c);
		z->compute_impulse_response();
	}
}	

fast_alg::complex_num
zak_channel_manager::get_coefficients( int rx_index, int tx_index ) 
{
	zak_channel* c = static_cast<zak_channel*>(channels[rx_index]);
	return c->get_coefficient( tx_index, sysp->delta_tau, sysp->delta_nu );
}

double
zak_channel_manager::first_order_isi( int rx_index, int tx_index )
{
	zak_channel* c = static_cast<zak_channel*>(channels[rx_index]);
	return c->get_isi( tx_index, sysp->delta_tau, sysp->delta_nu );
}

double
zak_channel_manager::higher_order_isi( int rx_index, int tx_index )
{
	zak_channel* c = static_cast<zak_channel*>(channels[rx_index]);
	return c->get_isi( tx_index, 2*sysp->delta_tau, 2*sysp->delta_nu );
}

std::vector<zak_component> 
zak_channel_manager::get_full_impulse( int rx_index, int tx_index )
{
	zak_channel* c = static_cast<zak_channel*>(channels[rx_index]);
	return c->get_zak_response( tx_index );
}
