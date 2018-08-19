
#include <functional>
#include <string>
#include <vector>
#include <sstream>

#include "plugin_manager.hxx"
#include "chat/chat_connection.hxx"
#include "chat/chat_factory.hxx"

using std::vector;
using std::string;
using std::ostringstream;
using ngincc::core::plugin_manager;
using ngincc::apps::chat::chat_factory;
using ngincc::apps::chat::connection_state;
using ngincc::apps::chat::connection_state_connected;
using ngincc::apps::chat::connection_state_logged_in;
using ngincc::apps::chat::connection_state_in_room;
using ngincc::apps::chat::connection_state_quitting;
using ngincc::apps::chat::chat_connection;

chat_factory::chat_factory(
    plugin_manager& core_plug
    , plugin_manager& chat_plug
    ) : connected_state(core_plug, chat_plug, *this)
    , logged_in_state(chat_plug)
    , in_room_state(chat_plug)
    , quitting_state(chat_plug)
     {
    std::function<int(vector<string>&, ostringstream&)> show_callback
        = [this] (vector<string>&,ostringstream&output) {
            // show each connection
            for(auto&& xclient : clients) {
                if(!xclient) {
                    continue;
                }
                // xclient->desc(output);
                output << xclient->to_string() << std::endl;
            }
            return 0;
    };
    core_plug.plug_add("chat/show", "It shows the chat objects", std::move(show_callback));
}

chat_factory::~chat_factory() {
    close_all();
}

int chat_factory::add_chat_connection(std::unique_ptr<chat_connection>& chat) {
    // find an empty slot
    bool added = false;
    for(auto&& xslot : clients) {
        if(!xslot) {
            xslot = std::move(chat);
            added = true;
            break;
        }
    }
    if(!added) {
        clients.emplace_back(std::move(chat));
    }
    return 0;
}

connection_state*chat_factory::get_connected() {
    return &connected_state;
}

connection_state*chat_factory::get_logged_in() {
    return &connected_state;
}

connection_state*chat_factory::get_in_room() {
    return &connected_state;
}

connection_state*chat_factory::get_quitting() {
    return &connected_state;
}

int chat_factory::close_all() {
    // softquit each connection
    for(auto&& xclient : clients) {
        if(!xclient) {
            continue;
        }
        xclient->close_handle(true);
        xclient = nullptr;
    }
    return 0;
}

