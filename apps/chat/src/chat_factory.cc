
#include <functional>
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>

#include "plugin_manager.hxx"
#include "chat/chat_connection.hxx"
#include "chat/chat_factory.hxx"

using std::vector;
using std::string;
using std::ostringstream;
using ngincc::core::plugin_manager;
using ngincc::core::event_loop;
using ngincc::net::raw_pipeline;
using ngincc::apps::chat::chat_factory;
using ngincc::apps::chat::connection_state;
using ngincc::apps::chat::connection_state_connected;
using ngincc::apps::chat::connection_state_logged_in;
using ngincc::apps::chat::connection_state_in_room;
using ngincc::apps::chat::connection_state_quitting;
using ngincc::apps::chat::chat_connection;

bool chat_factory::has_chat(long long token) {
    return (clients.size() > (unsigned long)token) && clients[token];
}

std::unique_ptr<chat_connection>& chat_factory::get_chat(long long token) {
    if(has_chat(token)) {
        return clients[token];
    }
    throw std::range_error("chat-connection not found");
}

std::unique_ptr<chat_connection>& chat_factory::create_chat_connection(int fd, connection_state*state) {
    // find an empty slot
    bool added = false;
    long long token = 0;
    for(unsigned int i = 0; i < clients.size(); i++) {
        if(!clients[i]) {
            clients[i].reset(new chat_connection(fd,i,eloop,raw_pipe,state));
            token = i;
            added = true;
            break;
        }
    }
    if(!added) {
        clients.emplace_back(new chat_connection(fd,clients.size(),eloop,raw_pipe,state));
        token = clients.size()-1;
    }
    return clients[token];
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

chat_factory::chat_factory(
    plugin_manager& core_plug
    , plugin_manager& chat_plug
    , event_loop& eloop
    , raw_pipeline& raw_pipe
    ) : eloop(eloop)
    , raw_pipe(raw_pipe)
    , connected_state(core_plug, chat_plug, *this)
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


