
#include <memory>
#include <functional>
#include <string>

#include "ngincc_config.h"
#include "log.hxx"
#include "plugin_manager.hxx"
#include "chat/chat_connection.hxx"
#include "chat/chat_factory.hxx"
#include "chat/help.hxx"

using std::string;
using std::vector;
using std::ostringstream;
using std::endl;

using ngincc::core::plugin_manager;
using ngincc::apps::chat::chat_connection;
using ngincc::apps::chat::connection_state;
using ngincc::apps::chat::help;
using namespace std::placeholders;


int help::process_help(vector<string>& cmd_args, chat_connection& chat) {
    ostringstream output;
    for(const auto x: chat_plug) {
        output << std::get<0>(x.second) << endl << '\t' << std::get<1>(x.second) << endl; 
    }

    chat.net_send(output.str(), 0);
	return 0;
}

help::help(plugin_manager& chat_plug) : chat_plug(chat_plug) {
    
    // make help command
    std::function<int(vector<string>&,chat_connection&)> help_cb = std::bind(&help::process_help, this, _1, _2);
    chat_plug.plug_add("state/connected/help", "Show help", std::move(help_cb));
    chat_plug.plug_add("state/logged_in/help", "Show help", std::move(help_cb));
    chat_plug.plug_add("state/in_room/help", "Show help", std::move(help_cb));
    chat_plug.plug_add("state/quitting/help", "Show help", std::move(help_cb));
}

help::~help() {
	// TODO composite_unplug_bridge(chat_plugin_manager_get(), 0, chat_help_plug);
}


