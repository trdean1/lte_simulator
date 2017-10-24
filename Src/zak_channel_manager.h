#include "channel_mananger.h"

class zak_channel_manager : public channel_manager {
	public:
		zak_channel_manager( base_station* bs, std::vector<user_equipment*> ues, params* sysp );

		fast_alg::complex_num get_coefficients( int rx_index, int tx_index );

		double first_order_isi( int rx_index, int tx_index );
		double higher_order_isi( int rx_index, int tx_index );

		std::vector<zak_component> get_full_impulse_response( int rx_index, int tx_index );
};
