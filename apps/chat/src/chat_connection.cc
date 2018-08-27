
#include <any>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <unistd.h>
#include <cstring> // defines strerror
#include <sys/socket.h> // defines send,recv

#include "ngincc_config.h"
#include "log.hxx"
#include "event_loop.hxx"
#include "plugin_manager.hxx"
#include "binary_coder.hxx"
#include "raw_pipeline.hxx"
#include "chat/chat_server_stack.hxx"
#include "chat/chat_connection.hxx"
#include "chat/chat_connection_state.hxx"
#include "net_channel.hxx"

using std::string;
using std::ostringstream;
using std::endl;
using std::vector;
using std::istream;
using ngincc::core::plugin_manager;
using ngincc::core::event_loop;
using ngincc::core::buffer_coder;
using ngincc::net::raw_pipeline;
using namespace ngincc::apps::chat;
using namespace std::placeholders;

/*
static int default_chat_send(struct streamio*strm, aroop_txt_t*content, int flags) {
	int ret = 0;
	struct chat_connection*chat = (struct chat_connection*)strm;
	if(chat->state & (CHAT_QUIT | CHAT_SOFT_QUIT | CHAT_ZOMBIE)) {
		return -1;
	} else {
		ret = real_send(strm, content, flags);
		if(strm->error) {
			chat->state |= CHAT_SOFT_QUIT;
		}
	}
	return ret;
}

static int default_chat_close(struct streamio*strm) {
	struct chat_connection*chat = (struct chat_connection*)strm;
	if(!chat->quited_at) {
		chat->quited_at = time(NULL);
	}
	if(!(chat->state & (CHAT_SOFT_QUIT | CHAT_QUIT))) {
		chat->state |= CHAT_QUIT;
	}
	if(chat->state & CHAT_IN_ROOM) {
		broadcast_room_leave(chat);
	}
	if(chat->state & CHAT_LOGGED_IN) {
		logoff_user(chat);
	}
	if(strm->bubble_up) {
		return strm->bubble_up->close(strm->bubble_up);
	}
	if(strm->fd != INVALID_FD) {
		struct chat_connection*chat = (struct chat_connection*)strm;
		event_loop_unregister_fd(strm->fd);
		if(!(chat->state & CHAT_SOFT_QUIT))close(strm->fd);
		strm->fd = INVALID_FD;
	}
	return 0;
}*/

int chat_connection::set_state(connection_state*next_state) {
    if(nullptr == next_state) {
        throw std::invalid_argument("state cannot be null");
    }
    state = next_state;
    return 0;
}

bool chat_connection::in_state(const std::type_info& given_id) const {
    return given_id == typeid(*state);
}

int chat_connection::set_login_name(const string& given) {
    login_name = given;
    return 0;
}

int chat_connection::on_recv(std::string&& content) {
    return state->process_chat_request(*this, content);
}


int chat_connection::on_recv(const std::string &content) {
    return state->process_chat_request(*this, content);
}


const string chat_connection::to_string() const {
    ostringstream output;
    // show name
    output << '\t' << login_name << '\t';
    // show state
	output << state->to_string();
    // TODO show if chained
    
    // if(channel.is_chained()) {output << "chained";} 
    // show if it is invalid-fd
    if(INVALID_FD == fd) {
        output << '\t' << "invalid fd";
    }
    return output.str();
}

chat_connection::chat_connection(
    int fd
    , long long token
    , event_loop& eloop
    , raw_pipeline& raw_pipe
    , connection_state*state
    )
    : default_net_channel(fd,eloop,raw_pipe)
    , fd(fd)
    , flags(0)
    , token(token)
    , eloop(eloop)
    , login_name("anonymous")
    , state(state)
    {
    if(INVALID_FD == fd || 0 == fd) {
        throw std::range_error("Invalid fild descriptor");
    }
	// chat_zombie_module_init();
}

chat_connection::~chat_connection() {
    close_handle();
	//chat_zombie_module_deinit();
	//composite_unplug_bridge(chat_plugin_manager_get(), 0, chat_factory_show);
	//pm_unplug_callback(0, chat_factory_on_softquit);
}

bool chat_connection::operator ==(const chat_connection& other) const {
    return (this == &other) || (login_name == other.login_name);
}

