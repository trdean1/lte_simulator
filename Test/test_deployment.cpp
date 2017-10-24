#include "map.h"
#include "path.h"
#include "user_equipment.h"
#include "base_station.h"
#include "params.h"

int main() {
	double step_ms = 50;

	params sysp;
	sysp.x_max = 200;
	sysp.y_max = 200;
	sysp.is_static = true;
	sysp.n_users = 4;
	sysp.n_bs_antennas = 4;
	sysp.array_theta = 0;
	sysp.array_delta = 0.1;

	map static_map ( &sysp );

	sysp.is_static = false;
	sysp.n_users = 16;
	sysp.num_paths = 10;
	sysp.v_mu = 30;
	sysp.v_sigma = 10;

	map mobile_map ( &sysp );

	//This shouldn't do anything but should still be valid
	for( int i = 0; i < 1000; i++ ) {
		static_map.update_locations( step_ms );
	}

	printf("Static:\n");
	static_map.print_locations();
	base_station* bs = static_map.get_bs();
	bs->print_elements();

	printf("Mobile:\n");
	mobile_map.print_locations();
	bs = mobile_map.get_bs();
	bs->print_elements();
	//This advances the simulation by 100 seconds
	for( int i = 0; i < 1000; i++ ) {
		mobile_map.update_locations( step_ms );
		//if( i % 50 == 0 )
		mobile_map.print_locations();
	}
}
