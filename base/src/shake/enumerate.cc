
#include <vector>
#include <iostream>
#include <sstream>
#include <unistd.h> // defines getpid()

#include "plugin_manager.hxx"
#include "shake/shake_intrprtr.hxx"
#include "shake/help_internal.hxx"
#include "parallel/pipeline.hxx"

using std::string;
using std::ostringstream;
using std::endl;
using std::vector;
using ngincc::core::plugin_manager;
using namespace ngincc::core::shake;


enumerate::enumerate(plugin_manager&pm) {
    accumulator = 0;
    std::function<int(vector<string>&, ostringstream&)> callback = [&] (vector<string> &cmd_args, ostringstream &output) {
        accumulator++;
        output << "[" << getpid() << "]Increased to " << accumulator << endl; 
        return 0;
    };
    pm.plug_add("shake/enumerate", "It increments an internal counter and displays it.", move(callback));
}

enumerate::~enumerate() {
    // TODO 
	// pm_unplug_callback(0, enumerate_command);
}

