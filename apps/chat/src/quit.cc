
#include <memory>
#include <functional>
#include <string>

#include "ngincc_config.h"
#include "log.hxx"
#include "plugin_manager.hxx"
#include "chat/chat_connection.hxx"
#include "chat/chat_factory.hxx"
#include "chat/quit.hxx"

using std::string;
using std::vector;
using std::ostringstream;

using ngincc::core::plugin_manager;
using ngincc::apps::chat::chat_connection;
using ngincc::apps::chat::connection_state;
using ngincc::apps::chat::quit;
using namespace std::placeholders;


int quit::process_quit(vector<string>& cmd_args, chat_connection& chat) {
	// TODO broadcast_room_leave(chat);
	// chat->set_state(chat, CHAT_QUIT);
    // TODO chat.set_state(factory.get_quitting);
	// TODO logoff_user(chat);
	chat.net_send("Bye\n", 0);
	return -1;
}

quit::quit(plugin_manager& chat_plug) : chat_plug(chat_plug) {
    
    // make quit command
    std::function<int(vector<string>&,chat_connection&)> quit_cb = std::bind(&quit::process_quit, this, _1, _2);
    chat_plug.plug_add("state/logged_in/quit", "Quit chat", std::move(quit_cb));
}

quit::~quit() {
	// TODO composite_unplug_bridge(chat_plugin_manager_get(), 0, chat_quit_plug);
}


