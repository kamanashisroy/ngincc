
#include <functional>
#include <string>
#include <sstream>
#include <iostream>

#include "ngincc_config.h"
#include "log.hxx"
#include "base_subsystem.hxx"
#include "net_subsystem.hxx"
//#include "db.hxx"


using ngincc::net::net_subsystem;

static int run_net() {
    net_subsystem net;
    net.parallel_init();
    net.run();
    return 0;
}

int main(int argc, char**argv) {
	//daemon(0,0);
	setlogmask (LOG_UPTO (LOG_NOTICE));
	openlog ("nginz_net", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    run_net();
	closelog();
	return 0;
}

/*static int nginz_main(char**args) {
	//daemon(0,0);
	setlogmask (LOG_UPTO (LOG_NOTICE));
	nginz_core_init();
	nginz_net_init();
	nginz_parallel_init();
	nginz_net_init_after_parallel_init();
	fiber_module_run();
	nginz_net_deinit();
	nginz_core_deinit();
	closelog();
	return 0;
}*/

