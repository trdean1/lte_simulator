#include "channel_manager.h"

void
channel_manager::channel_manager( )
{

}

void
channel_manager::~channel_manager()
{
	for( channel *c : channels ) 
		delete c;
}

void
channel_manager::update( double t )
{
	for( channel* c : channels ) {
		c->update_lsp_locat( t );
		c->update_ssp( );	
	}
}

double 
channel_manager::get_loss( int rx_index )
{
	return c[rx_index]->get_total_loss();
}
