#include "user_equipment.h"

user_equipment::user_equipment( double x, double y, double x_dot, double y_dot )
{
	x_location = x;
	y_location = y;
	x_velocity = x_dot;
	y_velocity = y_dot;
}

void
user_equipment::update_location( double time_ms )
{
	x_location += x_velocity * time_ms / 1000.0;
	y_location += y_velocity * time_ms / 1000.0;
}

void
user_equipment::set_location( double x, double y )
{
	x_location = x;
	y_location = y;
}
