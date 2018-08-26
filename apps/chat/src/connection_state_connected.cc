
#include <memory>
//#include <vector>
#include <functional>
#include <string>

#include "ngincc_config.h"
#include "log.hxx"
#include "module.hxx"
#include "plugin_manager.hxx"
#include "async_db.hxx"
#include "chat/chat_connection.hxx"
#include "chat/chat_connection_state.hxx"
#include "chat/chat_factory.hxx"

using std::string;
using std::ostringstream;
using std::istringstream;
using std::endl;
using std::vector;
using std::unique_ptr;
using std::stringstream;
using ngincc::core::plugin_manager;
using ngincc::core::buffer_coder;
using ngincc::core::binary_coder;
using ngincc::db::async_db;
using ngincc::apps::chat::chat_connection;
using ngincc::apps::chat::connection_state;
using ngincc::apps::chat::connection_state_connected;
using namespace std::placeholders;

#define CHAT_DEBUG(...) syslog(__VA_ARGS__)
//#define CHAT_DEBUG(...)

static inline const string USER_PREFIX {"chat/user/"};
static inline const string on_asyncchat_login {"on/asyncchat/login"};

connection_state_connected::connection_state_connected(
        plugin_manager& core_plug
        , plugin_manager& chat_plug
        , chat_factory& factory
        , async_db& adb_client
    ) : chat_plug(chat_plug) , factory(factory), adb_client(adb_client) {
    //std::function<int(vector<string>&,chat_connection&)> login_callback = std::bind(&chat_user::on_login_command, this, _1);
    //chat_plug.plug_add("state/connected/user", "It tries to set user name.", login_callback);
    std::function<int(buffer_coder&)> async_login_reply_callback = std::bind(&connection_state_connected::on_login_reply, this, _1);
    core_plug.plug_add(string(on_asyncchat_login), "It helps chat user to collect the hooks", std::move(async_login_reply_callback));
}


connection_state_connected::~connection_state_connected() {
}

int connection_state_connected::process_chat_request(
    chat_connection& chat, const string& request) {
    if(request.size() == 0) { // skip empty requests
        return 0;
    }
    if(request[0] == '/') { // check if it is a command
        CHAT_DEBUG(LOG_NOTICE, "processing command %s", request.c_str());
        istringstream command_reader(request);
        char discard_forward_slash;
        command_reader >> discard_forward_slash;

        std::vector<string> command_args;
        char delim;
        for(string token; command_reader >> token; command_reader >> delim) {
            if(token.size() == 0) {
                continue;
            }
            command_args.push_back(token);
        }
        if(command_args.size() > 0) {

            CHAT_DEBUG(LOG_NOTICE, "processing command ... %s", command_args[0].c_str());
            if(command_args[0] == "_welcome") { // say welcome
                CHAT_DEBUG(LOG_NOTICE, "saying hi");
                chat.net_send("Welcome to NginZ chat server\nLogin name?\n", 0);
                return 0;
            } else { // allow other commands
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
            }
        }
    } else { // try user log-in
        istringstream request_reader(request);
        string requested_name;
        request_reader >> requested_name;
        try_login(chat,requested_name);
    }
    return 0;
}

const string connection_state_connected::to_string() const {
    return "<connected>";
}

int connection_state_connected::try_login(chat_connection& chat, const string& name) {
    string response = "trying ..\n";
    if(name.size() == 0) {
        response = "invalid name :(, please try again.\n";
    } else if (name.size() >= NGINZ_MAX_CHAT_USER_NAME_SIZE) {
        response = "The name is too big\n";
    } else {
        async_try_login(name, (uint32_t)chat.get_token(), "on/asyncchat/login");
    }
    chat.net_send(response, 0);
	return 0;
}


int connection_state_connected::async_logoff(const string& name) {
    stringstream builder;
    builder << USER_PREFIX << name;
    adb_client.unset(-1, async_db::empty_hook, builder.str());
	return 0;
}

int connection_state_connected::async_try_login(const string& name, uint32_t token, string&& response_hook) {
	if(name.size() == 0) // sanity check
		return -1;

    stringstream builder;
    builder << USER_PREFIX << name;
    adb_client.set_if_null(token, on_asyncchat_login, builder.str(), token);
	return 0;
}


int connection_state_connected::on_login_reply(buffer_coder& recv_buffer) {
    uint32_t srcpid;
    string service;
    uint32_t reply_token;
    uint32_t reply_status;
    string name;
	// 0 = srcpid, 1 = reply_hook, 2 = reply_token, 3 = success, 4 = key, 5 = newvalue
    binary_coder coder(recv_buffer);

    // canary check
    string can_start;
    coder >>= can_start;
    if(can_start != binary_coder::canary_begin) {
        throw std::underflow_error("on_login_reply:canary check failed");
    }

    coder >>= srcpid;
    coder >>= service;
    coder >>= reply_token;
    coder >>= reply_status;
    coder >>= name;
    
	syslog(LOG_NOTICE, "[token%d]received:%d[value:%s]", reply_token, reply_status, name.c_str());
	if(name.size() == 0) {
		syslog(LOG_ERR, "Error, we do not know user name\n");
		return 0;
	}
    if(factory.has_chat(reply_token)) {
        unique_ptr<chat_connection>& chat = factory.get_chat(reply_token);
        if(chat->in_state(typeid(connection_state_connected))) {
            if(reply_status) {
                chat->set_login_name(name);

                // welcome user
                ostringstream output;
                output << "Welcome " << name << "!" << endl;
                chat->net_send(output.str(), 0);

                // user is logged in
                chat->set_state(factory.get_logged_in());
            } else {
                chat->net_send("Sorry, name is taken.\n", 0);
            }
        } else {
            async_logoff(name);
        }
    } else {
		syslog(LOG_ERR, "Error, could not find the logged in user, may be the user disconnected before login\n");
        async_logoff(name);
		return 0;
	}
	return 0;
}

