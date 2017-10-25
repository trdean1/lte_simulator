#include "map.h"

map::map( params* d_sysp )
 	: bs( d_sysp->x_max / 2.0, 
		  d_sysp->y_max / 2.0, 
		  d_sysp->n_bs_antennas, 
		  d_sysp->array_theta, 
		  d_sysp->array_delta )	
{
	sysp = d_sysp;

	if( sysp->is_static )
		init_static( sysp->x_max, sysp->y_max, sysp->n_users, sysp->n_bs_antennas,
					 sysp->array_theta, sysp->array_delta );
	else
		init_mobile( sysp->x_max, sysp->y_max, sysp->n_users,
   					 sysp->num_paths, sysp->v_mu, sysp->v_sigma,
					 sysp->n_bs_antennas, sysp->array_theta, sysp->array_delta );

	create_cm();
}

//Generate static map
void
map::init_static( double x, double y, uint32_t n_ue,
		  uint32_t n_bs_antenna, double theta, double spacing)
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

void
map::init_mobile( double x, double y, uint32_t n_ue, 
		  uint32_t n_paths, double v_mu, double v_sigma,
	      uint32_t n_bs_antenna, double theta, double spacing )
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


map::~map() 
{
	delete cm;
}

void
map::create_cm()
{
	if( sysp->is_tf_channel )
		cm = new tf_channel_manager( get_bs(), get_ue_list(), sysp );
	else
		cm = new zak_channel_manager( get_bs(), get_ue_list(), sysp );
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

std::vector<user_equipment*>
map::get_ue_list()
{
	std::vector<user_equipment*> out;
	for( int i = 0; i < ue_list.size(); i++ ) {
		out.push_back( &ue_list[i] );
	}
	return out;
}

void
map::update( double time_ms )
{
	update_locations( time_ms );
	update_channels( time_ms );
}

void 
map::update_locations( double time_ms )
{
	//If it is a static map, there are no paths
	if ( paths.empty() )
		return;

	for( int i = 0; i < ue_list.size(); i++ ) {
		ue_list[i].update_location( time_ms );

		//Check if we moved outside of the box, if so, wrap around to the other
		//side.
		if( ue_list[i].get_x() > x_dim || ue_list[i].get_x() < 0 ||
			ue_list[i].get_y() > y_dim || ue_list[i].get_y() < 0 ) {
			//This is a really lazy way to get to the correct side of the map.
			//If we pick the wrong side, then we'll end up here next iteration
			//and we can just hope we randomly end up on the other side.
			double p = round( randu( generator ) );
			std::pair<double,double> p_0 = paths[ path_map[i] ].get_point_from_param( p );
			ue_list[i].set_location( p_0.first, p_0.second );
		}
	}
}

void
map::update_channels( double t )
{
	cm->update_channels( t );
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
