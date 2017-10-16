#include <cmath>
#include <tuple>
#include <random>
#include <cassert>
#include <chrono>

#define _USE_MATH_DEFINES

#pragma once

class path {
	public:
		path( double, double, double min_len = 0.1 );
		
		std::pair<double, double> get_point_from_param(double);
		std::pair<double, double> get_unit_vector();
	private:
		double theta;
		double y_intercept;

		double m;
		double x_start;
		double x_stop;
		double y_start;
		double y_stop;
		double seg_length;

		std::default_random_engine generator;
  		std::uniform_real_distribution<double> randu;
};
