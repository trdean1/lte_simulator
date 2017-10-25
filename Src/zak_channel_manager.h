#include "channel_manager.h"
#include "zak_channel.h"
#include "user_equipment.h"
#include "base_station.h"
#include "params.h"

#pragma once

class zak_channel_manager : public channel_manager {
	public:
		zak_channel_manager( base_station* bs, std::vector<user_equipment*> ues, params* sysp );

		void update_channels( double t );

		fast_alg::complex_num get_coefficients( int rx_index, int tx_index );

		double first_order_isi( int rx_index, int tx_index );
		double higher_order_isi( int rx_index, int tx_index );

		std::vector<zak_component> get_full_impulse( int rx_index, int tx_index );
};
