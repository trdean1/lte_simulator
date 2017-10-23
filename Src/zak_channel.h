#include "channel.h"
#include "fast_alg.h"

#include <vector>
#include <unordered_map>

struct zak_component {
	double amplitude;
	double phase;
	double delay;
	double doppler;


};

size_t hash_zak( const zak_component& z )
{
	size_t out = 5381;
    out += 33 * (z.delay / 1e-9) + 1;
	out += 33 * (z.doppler / 1e-5 ) + 1;
	out += 33 * (z.amplitude / 1e-5 ) + 1;
	out += 33 * (z.phase / 1e-5 ) + 1;
	return out;
}

bool operator== (const zak_component& a, const zak_component& b)
{
	size_t ha = 5381;
	ha += 33 * (a.delay / 1e-9) + 1;
	ha += 33 * (a.doppler / 1e-5 ) + 1;
	ha += 33 * (a.amplitude / 1e-5 ) + 1;
	ha += 33 * (a.phase / 1e-5 ) + 1;

	size_t hb = 5381;
	hb += 33 * (b.delay / 1e-9) + 1;
	hb += 33 * (b.doppler / 1e-5 ) + 1;
	hb += 33 * (b.amplitude / 1e-5 ) + 1;
	hb += 33 * (b.phase / 1e-5 ) + 1;

	return ha == hb;
}

class zak_channel : public channel {
	public:
		using channel::channel;

		void compute_impulse_response( double t = 0 );
		std::vector<zak_component> get_zak_response( int element_index )
			{ return zak_response_per_element[element_index]; }

	private:
		
		std::vector<zak_component>
		merge_ambiguous_components( std::vector<zak_component> all_Zs, 
									double nu_res,
									double tau_res );

		std::vector<std::vector<zak_component>> zak_response_per_element;
};
