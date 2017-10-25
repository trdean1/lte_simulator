#include "channel.h"

#pragma once

class channel_manager{
	public:
		channel_manager();
		virtual ~channel_manager();

		virtual void update_channels( double t );
		double get_loss( int rx_index );

	protected:
		params* sysp;
		std::vector<channel *> channels;
};
