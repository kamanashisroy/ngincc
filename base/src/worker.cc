
#include <any>
#include <functional>
#include <tuple>
#include <stdexcept> // defines overflow_error
#include <cstring> // defines memset
#include <sstream>
#include <vector>
#include <syslog.h>

#include "ngincc_config.h"
#include "module.hxx"
#include "plugin_manager.hxx"
#include "worker.hxx"

using ngincc::core::plugin_manager;
using ngincc::core::worker;
using std::string;
using std::ostringstream;
using std::vector;

worker::worker(plugin_manager&pm) {
	memset(fibers, 0, sizeof(struct internal_fiber)*NGINCC_MAX_FIBERS);
    std::function<int(vector<string>&, ostringstream&)> callback = [this] (vector<string> &cmd_args, ostringstream &output) -> int {
            int count = 0;
            int i = 0;
            for(i = 0; i < NGINCC_MAX_FIBERS; i++) {
                if(fibers[i].status == worker::FIBER_STATUS_ACTIVATED) {
                    count++;
                    //syslog(LOG_NOTICE, "filber:%s:%d", fibers[i].filename, fibers[i].lineno);
                }
            }
            output << count << " fibers" << std::endl;
            return 0;
        };
    pm.plug_add<vector<string>&,ostringstream&>("shake/fiber","It will show the number of active fibers.",move(callback));
}

int worker::register_full(const std::function<int(int)> &fiber, const char*filename, int lineno) {
	int i = 0;
	for(i = 0; i < NGINCC_MAX_FIBERS; i++) {
		//syslog(LOG_NOTICE, "fiber:[%d][%d] .. registering fiber %s:%d\n", fibers[i].status, i, filename, lineno);
		if(fibers[i].status != FIBER_STATUS_EMPTY)
			continue;
		//syslog(LOG_NOTICE, "registering fiber \n");
		fibers[i].status = FIBER_STATUS_CREATED;
		fibers[i].fiber = fiber;
		fibers[i].filename = filename;
		fibers[i].lineno = lineno;
		return i;
	}
    throw std::overflow_error("Fiber capacity is full");
	return -1;
}

int worker::unregister(int fiber_id) {
    if(fiber_id < 0 || fiber_id >= NGINCC_MAX_FIBERS) { // sanity check
        return -1;
    }
    if(FIBER_STATUS_EMPTY != fibers[fiber_id].status) {
        fibers[fiber_id].fiber(FIBER_STATUS_DESTROYED);
        fibers[fiber_id].status = FIBER_STATUS_EMPTY;
    }
	return 0;
}

int worker::quit() {
	internal_quit = 1;
	return 0;
}

int worker::step() {
	int i = 0;
	for(i = 0; i < NGINCC_MAX_FIBERS; i++) {
		if(fibers[i].status == FIBER_STATUS_EMPTY || fibers[i].status == FIBER_STATUS_DEACTIVATED || fibers[i].status == FIBER_STATUS_DESTROYED)
			continue;
		int ret = fibers[i].fiber(fibers[i].status);
		if(ret == -1) {
			//syslog(LOG_NOTICE, "Destroying fiber :%s:%d\n", fibers[i].filename, fibers[i].lineno);
			unregister(i);
		} else if(ret == -2) {
			fibers[i].status = FIBER_STATUS_DEACTIVATED;
		}
		if(fibers[i].status == FIBER_STATUS_CREATED)
			fibers[i].status = FIBER_STATUS_ACTIVATED;
		//return 0;
	}
	return 0;
}

int worker::run() {
	while(!internal_quit) {
		step();
	}
	return 0;
}

