#include "channel.h"
#include "fast_alg.h"

#include <vector>
#include <unordered_map>

#pragma once

struct zak_component {
	double amplitude;
	double phase;
	double delay;
	double doppler;
};

class zak_channel : public channel {
	public:
		using channel::channel;

		void compute_impulse_response( double t = 0 );

		fast_alg::complex_num get_coefficient( int, double, double );
		double get_isi( int, double, double );
		std::vector<zak_component> get_zak_response( int element_index )
			{ return zak_response_per_element[element_index]; }

	private:
		
		std::vector<zak_component>
		merge_ambiguous_components( std::vector<zak_component> all_Zs, 
									double nu_res,
									double tau_res );

		std::vector<std::vector<zak_component>> zak_response_per_element;
};
