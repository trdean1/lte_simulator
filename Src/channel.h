#include "node.h"
#include "base_station.h"
#include "params.h"

#include <cmath>
#include <random>
#include <complex>
#include <chrono>
#include <algorithm>

#define _USE_MATH_DEFINES

struct lsp {
	double shadow;
	double K;
	double DS;
	double ASA;
	double ASD;
};

class channel {
	public:
		channel( node*, node*, params* sysp );

		void update_lsp_local( double t = 0 );
		void update_ssp();

		bool is_LoS() { return LoS; }
		double get_distance() { return distance; }
		double get_los_aoa() {return los_aoa; }
		double get_los_aod() {return los_aod; }
		double get_pathloss() { return pathloss; }
		double get_shadow() { return shadow_dB; }
		double get_K() { return K; }
		double get_DS() { return DS; }
		double get_ASA() { return ASA; }
		double get_ASD() { return ASD; }

		lsp get_current_lsp ();
		lsp get_last_lsp ();

		void set_current_lsp( lsp );

		std::vector<double> get_delays() { return delays; }
		std::vector<double> get_scaled_delays() { return scaled_delays; }
		std::vector<double> get_cluster_powers() { return cluster_powers; }
		std::vector<double> get_scaled_cluster_powers() 
			{ return scaled_cluster_powers; }

		std::vector<std::vector<double>> get_aoa() { return aoa; }
		std::vector<std::vector<double>> get_aod() { return aod; }

	protected:
		node* ue;
		node* bs;
		params* sysp;

		double distance;
		double distance_traveled;
		double los_aoa;
		double los_aod;

		int clusters;
		int rays;
		int n_bs_elements;

		//Large scale parameters
		bool LoS;
		double pathloss;

		double shadow_dB;
		double last_shadow;

		double K;
		double last_K;

		double DS;
		double last_DS;

		double ASA;
		double last_ASA;

		double ASD; 
		double last_ASD;

		//Small scale parameters
		std::vector<double> delays;
		std::vector<double> scaled_delays;

		std::vector<double> cluster_powers;
		std::vector<double> scaled_cluster_powers;
		double max_power;

		std::vector<std::vector<double>> aod;
		std::vector<std::vector<double>> aoa;

		std::vector<std::vector<double>> phases;

		std::vector<double> cluster_velocity;

		//These are correlated, N(0,1) random variables used to generate our 
		//coefficients
		double kn, dsn, asan, asdn, sfn;

		std::default_random_engine generator;
  		std::uniform_real_distribution<double> randu;
		std::normal_distribution<double> randn;

		//Used to update LSP
		void determine_LoS();
		void compute_pathloss();
		void compute_shadowfading();
		void compute_K();
		void compute_DS();
		void compute_ASD();
		void compute_ASA();
		void cross_correlate();

		//Used to update SSP
		void compute_delays();
		void compute_cluster_powers();
		void compute_aod();
		void compute_aoa();
		void generate_phases();
		void compute_cluster_doppler();
		

		std::pair<int,int> get_two_strongest_clusters();

		double degtorad = 180.0 / M_PI;

		double ray_basis[20] = 
		{ 0.0447,-0.0447,0.1413,-0.1413,0.2492,-0.2492,
		  0.3715,-0.3715,0.5129,-0.5129,0.6797,-0.6797,
		  0.8844,-0.8844,1.1481,-1.1481,1.5195,-1.5195,
		  2.1551,-2.1551 };
};
