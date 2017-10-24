#include "channel_manager.h"
#include "tf_channel.h"
#include "user_equipment.h"
#include "base_station.h"
#include "params.h"

#include <armadillo>

#pragma once

class tf_channel_manager : public channel_manager {
	public:
		tf_channel_manager(  base_station* bs, 
						     std::vector<user_equipment*> ues, 
							 params* sysp );
		double first_order_isi( int rx_index, int tx_index );
		double higher_order_isi( int rx_index, int tx_index );
		arma::cx_vec get_coefficients( int rx_index, int tx_index );

		std::vector<impulse_pair> get_full_impulse( int rx_index, int tx_index );
};
