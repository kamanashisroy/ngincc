
#include <any>
#include <functional>
#include <sstream>
#include <vector>

#include "module.hxx"
#include "plugin_manager.hxx"

using ngincc::core::module;
using ngincc::core::plugin_manager;
using std::ostringstream;
using std::string;
using std::endl;
using std::vector;

plugin_manager::plugin_manager() {
    std::function<int(vector<string>&, ostringstream&)> callback = [this] (vector<string> &cmd_args, ostringstream &output) int {
            // show header
            output << "---------------" << endl;
            output << "plugin-space" << '\t' << "desc" << '\t' << "source" << endl;
            output << "---------------" << endl;
            // dump plugins
            for(auto x: plugs) {
                output << std::get<0>(x.second) << '\t'; // plugin-space
                output << std::get<1>(x.second) << '\t'; // description
                // TODO output << std::get<3>(x->second) << '\t'; // module-name
                output << endl;
            }
            output << "---------------" << endl;
            return 0;
        };
    plug_add("shake/plugin", "It dumps the available plugins", move(callback));
}

plugin_manager::~plugin_manager() {
    // XXX WHAT TO DO HERE ?? 
}


int plugin_manager::plug_add_helper(const std::string&& plugin_space, const std::string&& desc, const std::any&& plug) {
    const unsigned long long hcode = hash_func(plugin_space);
    plugs.insert({hcode,std::make_tuple(plugin_space,desc,plug)});
    return 0;
}

