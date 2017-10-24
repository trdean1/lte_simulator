#include "channel_manager.h"

class tf_channel_manager : public channel_manager {
	public:
		tf_channel_manager(  base_station* bs, std::vector<user_equipment*> ues, params* sysp );
		
		arma::cx_vec get_coefficients( int rx_index, int tx_index );

		std::vector<impulse_pair> get_full_impulse( int rx_index, int tx_index );
};
