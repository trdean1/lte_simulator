#include "sysconfig.h"
#include "params.h"
#include "map.h"

#include <iostream>
#include <stdio.h>

int main()
{
	FILE *fp = fopen("/Users/tom/IoT_Sim/conf/zak_example.conf", "r");
	if( fp == nullptr) {
		std::cerr << "Unable to open file" << std::endl;
		return 1;
	}
	sysconfig conf ( fp );
	params sysp = conf.param_from_file();

	map m ( &sysp );
}
