#include "base_station.h"

base_station::base_station( double x, double y, int n_elements,
			    			double theta, double spacing )
{
	x_location = x;
	y_location = y;

	n_antennas = n_elements;

	//Layout antenna elements
	double length = spacing * n_elements;
	double offset_x = length * cos(theta) / 2.0;
	double offset_y = length * sin(theta) / 2.0;
	double step_x = spacing * cos(theta);
	double step_y = spacing * sin(theta);

	for( int i = 0; i < n_elements; i++ ) {
		element_locations.push_back( { i*step_x - offset_x + x, 
									   i*step_y - offset_y + y} );
	}
}

void
base_station::print_elements()
{
	uint32_t i = 0;
	for( auto it = element_locations.begin(); it != element_locations.end(); it++ )
	{
		printf("Antenna %d: %f, %f\n", i++, it->first, it->second);
	}
}
