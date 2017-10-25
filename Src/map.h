#include "user_equipment.h"
#include "base_station.h"
#include "path.h"
#include "params.h"
#include "channel.h"
#include "channel_manager.h"
#include "zak_channel_manager.h"
#include "tf_channel_manager.h"

#include <unordered_map>
#include <vector>
#include <random>
#include <chrono>

#pragma once

class map {
	public:
		map( params* d_sysp );
		
		~map();

		void add_ue( user_equipment );
		void add_ue( double, double, double, double );
		void set_bs( base_station );

		void update( double );	//calls the two funcitons below
		void update_locations( double ); //updates all ue locations
		void update_channels( double ); //updates all channels 
		void print_locations();

		std::vector<user_equipment*> get_ue_list( );
		base_station* get_bs() { return &bs; }

	private:
		void create_cm();

		void init_static( double, double, uint32_t,
		  	 uint32_t n_bs_antenna, double theta, 
		  	 double spacing);						   //Generates static map
		void init_mobile( double, double, uint32_t, uint32_t, double, double,
		     uint32_t n_bs_antenna, double theta, 
		     double spacing ); //Generates mobile map
		
		params* sysp;

		std::vector<user_equipment> ue_list;
		base_station bs;
		std::vector<path> paths;

		channel_manager *cm;

		//Tells which path is assigned to each ue. 
		//path_map[i] = j means that ue_list[i] is assigned to paths[j]
		std::unordered_map<uint32_t, uint32_t> path_map; 

		double x_dim;
		double y_dim;

		std::default_random_engine generator;
  		std::uniform_real_distribution<double> randu;
		std::normal_distribution<double> randn;
};
