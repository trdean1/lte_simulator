#include "map.h"
#include "path.h"
#include "user_equipment.h"
#include "base_station.h"

int main() {
	double step_ms = 100;

	map static_map ( 200, 200, 4 );
	map mobile_map ( 200, 200, 16, 3, 30, 10 );

	//This shouldn't do anything but should still be valid
	for( int i = 0; i < 1000; i++ ) {
		static_map.update_locations( step_ms );
	}

	static_map.print_locations();

	mobile_map.print_locations();
	//This advances the simulation by 100 seconds
	for( int i = 0; i < 1000; i++ ) {
		mobile_map.update_locations( step_ms );
		if( i % 50 == 0 )
			mobile_map.print_locations();
	}
}
