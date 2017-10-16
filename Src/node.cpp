#include "node.h"

double
node::get_distance( node* n )
{
	double dx = x_location - n->get_x();
	double dy = y_location - n->get_y();

	return sqrt( dx*dx + dy*dy );
}
