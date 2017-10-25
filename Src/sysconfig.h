#include "params.h"

#include <stdio.h>
#include <iostream>
#include <libconfig.h++>

#pragma once

class sysconfig {
	public:
		sysconfig (FILE *fp);
		params param_from_file();

	private:
		libconfig::Config conf;
		params sysp;
};


