
#include <functional>
#include <string>
#include <sstream>
#include <iostream>

#include "ngincc_config.h"
#include "log.hxx"
#include "base_subsystem.hxx"
//#include "db.hxx"


using ngincc::core::base_subsystem;

static int run_base() {
    base_subsystem base;
    base.parallel_init();
    base.run();
    return 0;
}

int main(int argc, char**argv) {
	//daemon(0,0);
	setlogmask (LOG_UPTO (LOG_NOTICE));
	openlog ("nginz_base", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    run_base();
	closelog();
	return 0;
}

