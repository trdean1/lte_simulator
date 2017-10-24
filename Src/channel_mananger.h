#include "channel.h"

class channel_manager{
	public:
		channel_mananger();
		virtual ~channel_manager();

		void update_channels( double t );
		double get_loss( int rx_index );

	private:
		std::vector<channel *> channels;
};
