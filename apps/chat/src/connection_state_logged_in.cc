
#include <string>

#include "module.hxx"
#include "plugin_manager.hxx"
#include "chat/chat_connection.hxx"
#include "chat/chat_connection_state.hxx"

using std::string;
//using std::ostringstream;
using std::istringstream;
using std::endl;
//using std::vector;
using ngincc::core::plugin_manager;
using ngincc::apps::chat::chat_connection;
using ngincc::apps::chat::connection_state;
using ngincc::apps::chat::connection_state_logged_in;
//using namespace std::placeholders;

connection_state_logged_in::connection_state_logged_in(plugin_manager& chat_plug)
    : chat_plug(chat_plug) {
}

connection_state_logged_in::~connection_state_logged_in() {
}

int connection_state_logged_in::process_chat_request(
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
        for(string token;std::getline(command_reader,token,command_reader.widen(' '));) {
            if(token.size() == 0) {
                continue;
            }
            command_args.push_back(token);
        }

        // read the last token
        string last_token;
        command_reader >> last_token;
        if(last_token.size()) {
            command_args.push_back(last_token);
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
        return chat_plug.plug_call(request, std::tie(command_args,*this));
    } else { // try user log-in
        // what to do here ? show help ?
        chat.send("TODO Show a list of rooms", 0);
    }
    return 0;
}

const string connection_state_logged_in::to_string() const {
    return "<logged-in>";
}

