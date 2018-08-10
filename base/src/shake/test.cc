
#include <vector>
#include <iostream>
#include <sstream>

#include "plugin_manager.hxx"
#include "shake/shake_intrprtr.hxx"
#include "shake/test_internal.hxx"

using std::string;
using std::ostringstream;
using std::endl;
using std::vector;
using ngincc::core::plugin_manager;
using namespace ngincc::core::shake;

test::test(plugin_manager&pm) {
    std::function<int(vector<string>&, ostringstream&)> callback = [&,pm] (vector<string> &command, ostringstream& output) {
        // TODO pm.plug_wild_call("test/*");
        output << endl << "Tests Complete" << endl;
	    return 0;
    };
    pm.plug_add("shake/test", "It tests all the test cases.", move(callback));
}

test::~test() {
    // TODO 
	// pm_unplug_callback(0, test_command);
}

