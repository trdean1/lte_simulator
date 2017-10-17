#include <cmath>

#pragma once

class node {
	public:
		double get_x () { return x_location; }
		double get_y () { return y_location; }
		double get_x_dot () { return x_velocity; }
		double get_y_dot () { return y_velocity; }

		double get_distance( node* n );
		double get_los_aoa( node* n );
		double get_los_aod( node* n );

	protected:
		double x_location;
		double y_location;

		double x_velocity;
		double y_velocity;
};
