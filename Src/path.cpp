#include "path.h"

/*** Generate a random line that lies within [0,x_max) and [0,y_max)
 *	The minimum length must be min_len*sqrt(x**2 + y**2)
 *
 */
path::path( double x_max, double y_max, double min_len ) 
{
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	generator.seed( seed );
	double max_len = sqrt( x_max*x_max + y_max*y_max );
	while( true ) {
		theta = 2*M_PI*randu( generator );
		double y_int_max = x_max * tan( theta );
		y_intercept = y_int_max * randu( generator );
		
		//find length of line segment in box
		m = atan( theta );
		double x_int = -1*y_intercept / m;
		double x_max_int = m * x_max + y_intercept;

		//Find the lowest values of x and y in the box
		if( y_intercept > 0 && y_intercept < y_max ) {
			x_start = 0;
			y_start = y_intercept;	
		} else if (y_intercept < 0 ) {
			x_start = x_int;
			y_start = 0;
		} else {
			x_start = m*x_max_int + y_intercept;
			y_start = m*x_max + y_intercept;
		}

		//Find the largest values of x and y in the box
		if( m*x_max_int + y_intercept > y_max ) {
			x_stop = x_max;
			y_stop = m*x_max + y_intercept;
		} else {
			x_stop = m*x_max_int + y_intercept;
			y_stop = y_max;
		}

		seg_length = sqrt( pow(x_stop - x_start, 2) + pow(y_stop - y_start,2) );

		if( seg_length > min_len * max_len )
			break;
	}
}

/** Returns a pair {x,y} that found from the parametric representation 
 * of the line in the box.  
 *
 * t must lie between [0,1]
 */
std::pair<double, double> 
path::get_point_from_param( double t )
{
	assert( t <= 1 && t >= 0 );

	std::pair<double,double> p;
	p.first = x_start + t * (x_stop - x_start);
	p.second = y_start + t * (y_stop - y_start);

	return p;
}

std::pair<double, double> 
path::get_unit_vector()
{
	return { cos( theta ), sin( theta ) };
}
