
#include <vector>
#include <iostream>
#include <sstream>

#include "module.hxx"
#include "plugin_manager.hxx"
#include "shake/help_internal.hxx"

using std::string;
using std::ostringstream;
using std::endl;
using std::vector;
using ngincc::core::plugin_manager;
using namespace ngincc::core::shake;

help::help(plugin_manager&pm) {
    std::function<int(vector<string>&, ostringstream&)> callback = [&,pm](vector<string> &command,ostringstream &output) -> int {
        // TODO pm.plug_wild_call("shake/*");
        output << "TODO help" << endl;
        return 0;
    };
    pm.plug_add("shake/help", "It displays the command usage.", move(callback));
}

help::~help() {
    // TODO
	// pm_unplug_callback(0, help_command);
}

