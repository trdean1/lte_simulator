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
		theta = M_PI*randu( generator );
		m = tan( theta );
		double y_int_max;
		if( theta < M_PI / 2 ) {
			y_int_max = 200 + x_max * m;
			y_intercept = y_int_max * randu( generator ) - x_max * m;
		} else {
			y_int_max = -1.0 * x_max * m;
			y_intercept = y_int_max * randu( generator );
		}
		
		//find length of line segment in box
		double x_intercept = -1*y_intercept / m;
		//double x_max_int = m * x_max + y_intercept;

		//Find the lowest values of x and y in the box
		if( y_intercept > 0 && y_intercept < y_max ) {
			//In this case x=0 is in the box
			x_start = 0;
			y_start = y_intercept;	

			if( m*x_max + y_intercept > y_max ) {
				y_stop = y_max;
				x_stop = (y_max - y_intercept) / m;
			} else if( m*x_max + y_intercept < 0 ) {
				y_start = 0;
				y_stop = y_intercept;
				x_stop = x_intercept;
			} else {
				x_stop = x_max;
				y_stop = m*x_max + y_intercept;
			}
		} else if (y_intercept < 0 ) {
			//in this case, x=0 is below the box, but we are 
			//guaranteed that y=0 is in the box
			x_start = x_intercept;
			y_start = 0;

			if( m*x_max + y_intercept > y_max ) {
				y_stop = y_max;
				x_stop = (y_max - y_intercept) / m;
			} else {
				x_stop = x_max;
				y_stop = m*x_max + y_intercept;
			}
		} else {
			//in this case x=0 is above the box.  we know that
			//y=y_max is in the box
			y_stop = y_max;
			x_start = (y_stop - y_intercept) / m;

			if( m*x_max + y_intercept > 0 ) {
				x_stop = x_max;
				y_start = m*x_max + y_intercept;
			} else {
				y_start = 0;
				x_stop = x_intercept;
			}
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
