#include "node.h"

#pragma once

#include <cmath>
#include <vector>
#include <tuple>
#include <stdio.h>

class base_station : public node {
	public:
		base_station( double x, double y, 
					  int n_elements = 1, double theta = 0,
				      double spacing = 0	);

		int n_elements() { return n_antennas; }
		std::vector<std::pair<double,double>> get_elements() {return element_locations;}

		void print_elements();
	private:
		int n_antennas;

		std::vector<std::pair<double,double>> element_locations;
};
