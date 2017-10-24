#include "channel_manager.h"

channel_manager::channel_manager( )
{

}

channel_manager::~channel_manager()
{
	for( channel *c : channels ) 
		delete c;
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
