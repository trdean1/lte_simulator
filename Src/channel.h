#include "node.h"
#include "params.h"

#include <cmath>
#include <random>
#include <chrono>

class channel {
	public:
		channel( node*, node*, params* sysp );

		void update();

		bool is_LoS() { return LoS; }
		double get_distance() { return distance; }
		double get_pathloss() { return pathloss; }
		double get_shadow() { return shadow_dB; }
		double get_K() { return K; }
		double get_DS() { return DS; }
		double get_ASA() { return ASA; }
		double get_ASD() { return ASD; }

	private:
		node* ue;
		node* bs;
		params* sysp;

		double distance;

		bool LoS;
		int clusters;
		int rays;

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

		std::default_random_engine generator;
  		std::uniform_real_distribution<double> randu;
		std::normal_distribution<double> randn;

		void determine_LoS();
		void compute_pathloss();
		void compute_shadowfading();
		void compute_K();
		void compute_DS();
		void compute_ASD();
		void compute_ASA();
		void cross_correlate();
};
