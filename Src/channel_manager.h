#include "channel.h"
#include "user_equipment.h"
#include "base_station.h"
#include "params.h"

class channel_manager {
	public:
		channel_manager( std::vector<user_equipment>*, base_station*, params* );

		void update( double t );

	private:
		void update_channel_lsp( double t );
		void apply_spatial_consistancy( double t );
		void update_channel_ssp( double t );
		void update_channel_coefficients( double t );

		double last_update_time;
};
