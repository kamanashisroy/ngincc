
#include <string>

#include "module.hxx"
#include "plugin_manager.hxx"
#include "chat/chat_connection.hxx"
#include "chat/chat_connection_state.hxx"
#include "chat/broadcast_room.hxx"

using std::string;
//using std::ostringstream;
using std::istringstream;
using std::endl;
//using std::vector;
using ngincc::db::async_db;
using ngincc::core::plugin_manager;
using ngincc::apps::chat::chat_connection;
using ngincc::apps::chat::connection_state;
using ngincc::apps::chat::connection_state_in_room;
using ngincc::apps::chat::chat_factory;
//using namespace std::placeholders;

connection_state_in_room::connection_state_in_room(
    plugin_manager& chat_plug
    , chat_factory& factory
    , async_db& adb_client
    , broadcast_room_module& bcast_module)
    : chat_plug(chat_plug), factory(factory), adb_client(adb_client), bcast_module(bcast_module) {
}

connection_state_in_room::~connection_state_in_room() {
}

int connection_state_in_room::process_chat_request(
    chat_connection& chat, const string& request
    ) {
    if(request.size() == 0) { // skip empty requests
        return 0;
    }
    if(request[0] == '/') { // check if it is a command
        istringstream command_reader(request);
        char discard_forward_slash;
        command_reader >> discard_forward_slash;

        std::vector<string> command_args;
        for(string token; command_reader >> token;) {
            if(token.size() == 0) {
                continue;
            }
            command_args.push_back(token);
        }
        if(command_args[0][0] == '_') {
            // do not allow system command
            return 0;
        }
        string request("state/connected/"+command_args[0]);
        /*std::tuple<std::reference_wrapper<vector<string>>, std::reference_wrapper<chat_connection> > args(command_args,*this);
        return chat_plug.plug_call<vector<string>&,chat_connection&>(request, std::move(command_args), args);*/
        /*return chat_plug.plug_call<vector<string>&,chat_connection&>(
            request
            , std::move(command_args)
            , std::tie(command_args,*this)
        );*/
        return chat_plug.plug_call(request, std::tie(command_args,chat));
    } else { // try user log-in
        // what to do here ? show help ?
        bcast_module.send_all(chat, request);
    }
    return 0;
}

const string connection_state_in_room::to_string() const {
    return "<logged-in>";
}

