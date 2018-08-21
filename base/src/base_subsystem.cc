

#include <functional>
#include <string>
#include <sstream>
#include <iostream>

#include "ngincc_config.h"
#include "log.hxx"
#include <signal.h>
#include "base_subsystem.hxx"
//#include "db.hxx"

using std::string;
using std::ostringstream;
using std::vector;
using namespace ngincc::core;

static base_subsystem *base = nullptr;

worker& base_subsystem::get_worker() {
    return base_worker;
}

int base_subsystem::rehash() {
    ostringstream output;
    vector<string> command_args;
    base_plug.plug_call<std::vector<string>&,ostringstream& >(move(string("shake/rehash")),{command_args,output});
    // output to cout
    std::cout << output.str() << std::endl;
	return 0;
}

int base_subsystem::master_init() {
    ostringstream output;
    vector<string> command_args;
    base_plug.plug_call<std::vector<string>&,ostringstream& >(std::move(std::string("master/init")),{command_args,output});
    // output to cout (enable quiet option)
    std::cout << output.str() << std::endl;
	return 0;
}

static void signal_callback(int sigval) {
	base->get_worker().quit();
}

int base_subsystem::parallel_init() {
	rehash();
	signal(SIGPIPE, SIG_IGN); // avoid crash on sigpipe
	signal(SIGINT, signal_callback);
	fork_process(NGINZ_NUMBER_OF_PROCESSORS);

	// Setup for master
    if(base_pipe.is_master()) {
	    master_init();
    }
	return 0;
}

base_subsystem::base_subsystem()
    : base_plug()
    , base_worker(base_plug)
    , base_lazy(base_worker)
    , base_event_loop(base_worker,base_plug)
    , base_pipe(base_plug,base_event_loop)
    , base_shake(base_plug,base_worker,base_event_loop,base_pipe)
    {
    if(!base) {
        base = this;
    }
    binary_coder::self_test();
}

base_subsystem::~base_subsystem() {
    base = nullptr;
}

int base_subsystem::run() {
    base_worker.run();
    return 0;
}


