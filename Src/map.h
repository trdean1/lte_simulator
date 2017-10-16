#include "user_equipment.h"
#include "base_station.h"
#include "path.h"

#include <unordered_map>
#include <vector>
#include <random>
#include <chrono>

#pragma once

class map {
	public:
		//map( );
		map( double, double, uint32_t,
		  	 uint32_t n_bs_antenna, double theta, 
		  	 double spacing);						   //Generates static map
		map( double, double, uint32_t, uint32_t, double, double,
		     uint32_t n_bs_antenna, double theta, 
		     double spacing ); //Generates mobile map

		void add_ue( user_equipment );
		void add_ue( double, double, double, double );
		void set_bs( base_station );

		void update_locations( double );

		void print_locations();

		std::vector<user_equipment>& get_ue_list( ) { return ue_list; }
		base_station& get_bs() { return bs; }

	private:
		std::vector<user_equipment> ue_list;
		base_station bs;
		std::vector<path> paths;

		//Tells which path is assigned to each ue. 
		//path_map[i] = j means that ue_list[i] is assigned to paths[j]
		std::unordered_map<uint32_t, uint32_t> path_map; 

		double x_dim;
		double y_dim;

		std::default_random_engine generator;
  		std::uniform_real_distribution<double> randu;
		std::normal_distribution<double> randn;
};
