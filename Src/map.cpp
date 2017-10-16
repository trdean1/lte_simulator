#include "map.h"

//Generate static map
map::map( double x, double y, uint32_t n_ue,
		  uint32_t n_bs_antenna, double theta, double spacing)
 	: bs( x / 2.0, y / 2.0, n_bs_antenna, theta, spacing )	
{
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	generator.seed( seed );
	for( int i = 0; i < n_ue; i++ ) {
		//Place UE at random
		double xr =x*randu(generator);
		double yr =y*randu(generator);
		ue_list.emplace_back( xr, yr, 0, 0 );
	}

	x_dim = x;
	y_dim = y;
}

map::map( double x, double y, uint32_t n_ue, 
		  uint32_t n_paths, double v_mu, double v_sigma,
	      uint32_t n_bs_antenna, double theta, double spacing )
	: bs( x / 2.0, y / 2.0, n_bs_antenna, theta, spacing )
{
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	generator.seed( seed );
	std::uniform_int_distribution<int> randint (0, n_paths - 1);

	//Generate n paths along which UEs will be placed
	for( int i = 0; i < n_paths; i++ ) {
		paths.emplace_back( x, y );
	}	

	for( int i = 0; i < n_ue; i++ ) {
		int j = randint(generator);
		path_map[i] = j;
		double t = randu(generator);
		std::pair<double,double> pos = paths[j].get_point_from_param( t );
		double v_mag = v_sigma * randn( generator ) + v_mu;
		std::pair<double,double> v_hat = paths[j].get_unit_vector();
		ue_list.emplace_back( pos.first, pos.second, 
						      v_mag * v_hat.first, v_mag * v_hat.second );
	}

	x_dim = x;
	y_dim = y;
}

void 
map::add_ue( user_equipment ue ) {
	ue_list.push_back( ue );
}

void 
map::add_ue( double x, double y, double x_dot, double y_dot ) {
	ue_list.emplace_back( x, y, x_dot, y_dot );
}

void 
map::set_bs( base_station m_bs ) {
	bs = m_bs;
}

void 
map::update_locations( double time_ms )
{
	//If it is a static map, there are not paths
	if ( paths.empty() )
		return;

	for( int i = 0; i < ue_list.size(); i++ ) {
		ue_list[i].update_location( time_ms );

		//Check if we moved outside of the box, if so, wrap around to the other
		//side.
		if( ue_list[i].get_x() > x_dim || ue_list[i].get_x() < 0 ||
			ue_list[i].get_y() > y_dim || ue_list[i].get_y() < 0 ) {
			std::pair<double,double> p_0 = paths[ path_map[i] ].get_point_from_param( 0 );
			ue_list[i].set_location( p_0.first, p_0.second );
		}
	}
}

void
map::print_locations( )
{
	for( int i = 0; i < ue_list.size(); i++ ) {
		printf("%d: %f, %f; v = %f, %f; path = %d\n", i, 
				ue_list[i].get_x(), ue_list[i].get_y(),
			    ue_list[i].get_x_dot(), ue_list[i].get_y_dot(),
			    path_map[i] );
	}
	printf("\n");
}
