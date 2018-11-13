#include "channel.h"
#include "phy.h"

#include <armadillo>
#include <string>

#pragma once

class channel_manager{
	public:
		channel_manager();
		virtual ~channel_manager();

		virtual void update_channels( double t );
		double get_loss( int rx_index );

		void generate_qams( );
		void convolve_qams( );
		void output_qams( std::string filename );
		void output_output( std::string filename );

	protected:
		params* sysp;
		std::vector<channel *> channels;
		phy* p;

		arma::cx_cube qams;
		arma::cx_cube rx_signal;
};
