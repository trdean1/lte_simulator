#include "node.h"

double
node::get_distance( node* n )
{
	double dx = x_location - n->get_x();
	double dy = y_location - n->get_y();

	return sqrt( dx*dx + dy*dy );
}

double 
node::get_los_aoa( node* n )
{
	return atan( (x_location - n->get_x()) / (y_location - n->get_y()) );
}

double 
node::get_los_aod( node* n )
{
	return atan( (y_location - n->get_y()) / (x_location - n->get_x()) );
}
