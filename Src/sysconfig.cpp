#include "sysconfig.h"

sysconfig::sysconfig( FILE* fp )
{
	try {
		conf.read( fp );
	} catch(const libconfig::FileIOException &fioex) {
		std::cerr << "I/O error while reading file." << std::endl;
	} catch(const libconfig::ParseException &pex) {
		std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
             	  << " - " << pex.getError() << std::endl;
  	}

	conf.setAutoConvert(true);
}

params
sysconfig::param_from_file()
{
	try {
		sysp.f_c = conf.lookup("f_c");
	} catch(const libconfig::SettingNotFoundException &nfex) {
		std::cerr << "No 'f_c' setting in configuration file." << std::endl;
	}

	try {
		sysp.is_tf_channel = conf.lookup("is_tf_channel");
	} catch(const libconfig::SettingNotFoundException &nfex) {
		std::cerr << "No 'is_tf_channel' setting in configuration file." << std::endl;
	}

	if( sysp.is_tf_channel ) {
		if( !conf.lookupValue( "f_N", sysp.f_N ) )
			sysp.f_N = 3.2e7;
		if( !conf.lookupValue( "N", sysp.N ) )
			sysp.N = 16;
		if( !conf.lookupValue( "samp_per_symb", sysp.samp_per_symb ) )
			sysp.samp_per_symb = 8;
		if( !conf.lookupValue( "impulse_width", sysp.impulse_width ) )
			sysp.impulse_width = 0.5;
		if( !conf.lookupValue( "block_len", sysp.block_len ) )
			sysp.block_len = 256;
	} else {
		if( !conf.lookupValue( "N", sysp.N ) )
			sysp.N = 16;
		if( !conf.lookupValue( "delta_tau", sysp.delta_tau ) )
			sysp.delta_tau = 16;
		if( !conf.lookupValue( "delta_nu", sysp.delta_nu ) )
			sysp.delta_nu = 16;
		if( !conf.lookupValue( "zak_aspect", sysp.zak_aspect ) )
			sysp.zak_aspect = 1;
		if( !conf.lookupValue( "nu_resolution", sysp.nu_resolution ) )
			sysp.nu_resolution = 1;
		if( !conf.lookupValue( "tau_resolution", sysp.tau_resolution ) )
			sysp.tau_resolution = 1;
	}

	if( !conf.lookupValue( "x_max", sysp.x_max ) )
		sysp.x_max = 500;
	if( !conf.lookupValue( "y_max", sysp.y_max ) )
		sysp.y_max = 500;
	if( !conf.lookupValue( "n_users", sysp.n_users ) )
		sysp.n_users = 4;
	if( !conf.lookupValue( "n_bs_antennas", sysp.n_bs_antennas ) )
		sysp.n_bs_antennas = 1;
	if( !conf.lookupValue( "array_theta", sysp.array_theta ) )
		sysp.array_theta = 0;
	if( !conf.lookupValue( "array_delta", sysp.array_delta ) )
		sysp.array_delta = 0.1;

	try {
		sysp.is_static = conf.lookup("is_static");
	} catch(const libconfig::SettingNotFoundException &nfex) {
		std::cerr << "No 'is_static' setting in configuration file." << std::endl;
	}

	if( !sysp.is_static ) {
		if( !conf.lookupValue( "num_paths", sysp.num_paths ) )
			sysp.num_paths = 4;
		if( !conf.lookupValue( "v_mu", sysp.v_mu ) )
			sysp.v_mu = 10;
		if( !conf.lookupValue( "v_sigma", sysp.v_sigma ) )
			sysp.v_sigma = 3;
		if( !conf.lookupValue( "v_cluster_sigma", sysp.v_cluster_sigma ) )
			sysp.v_cluster_sigma = 0;
	}

	if (!conf.lookupValue( "modulation_order", sysp.modulation_order ) )
		sysp.modulation_order = 2;
	if (!conf.lookupValue( "tx_pow", sysp.tx_pow ) )
		sysp.tx_pow = 0;
	if (!conf.lookupValue( "rx_noise", sysp.rx_noise ) )
		sysp.rx_noise = -60;

	return sysp;	
}
