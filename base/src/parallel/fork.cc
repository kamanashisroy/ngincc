
#include <iostream>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <cstring>

#include "log.hxx"
#include "plugin_manager.hxx"
#include "base_subsystem.hxx"

using std::string;
using std::ostringstream;
using std::vector;
using ngincc::core::plugin_manager;
using ngincc::core::base_subsystem;


int base_subsystem::fork_process(int nclients) {
	if(nclients == 0)
		return 0;
	nclients--;
	pid_t pid = 0;
    ostringstream output;
    vector<string> command_args;
	base_plug.plug_call<int>("fork/before", {getpid()}); // called many times
	pid = fork();
	if(pid > 0) { // parent
		base_plug.plug_call<int>("fork/parent/after", {pid});
		fork_process(nclients); // fork more
	} else if(pid == 0) { // child
		base_plug.plug_call<int>("fork/child/after", {getpid()});
	} else {
		syslog(LOG_ERR, "Failed to fork:%s\n", strerror(errno));
		return -1;
	}
	return 0;
}


