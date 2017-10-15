#include "node.h"

#pragma once

class user_equipment : public node {
	public:
		//user_equipment( );
		user_equipment( double, double, double, double );

		void update_location( double );
		void set_location( double, double );
};
