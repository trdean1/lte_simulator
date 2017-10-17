#include "channel.h"
#include "fast_alg.h"

//Complex_num is coefficient, double is delay relative to smallest cluster delay
typedef std::pair<fast_alg::complex_num, double> impulse_pair;

class tf_channel : public channel {
	public:
		using channel::channel;

		std::vector<impulse_pair> get_element_impulse( int i )
			{ return impulse_response_per_element[i]; }

		void compute_impulse_response( double t );
	private:
		static bool impulse_comp( impulse_pair, impulse_pair );

		std::vector<std::vector<impulse_pair>> impulse_response_per_element;
};
