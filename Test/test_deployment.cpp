#include "map.h"
#include "path.h"
#include "user_equipment.h"
#include "base_station.h"

int main() {
	double step_ms = 50;

	map static_map ( 200, 200, 4, 4, 0, 0.1 );
	map mobile_map ( 200, 200, 16, 10, 30, 10, 16, 0, 0.1 );

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
