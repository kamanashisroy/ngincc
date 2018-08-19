
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

int chat_connection::on_client_data_helper() {
	if(recv_buffer.in_avail() == 0) {
		return 0;
	}
    istream reader(dynamic_cast<std::basic_streambuf<char>*>(&recv_buffer));
    // NOTE we require newline at the end of data
    // TODO fix this to allow a stream or partial lines
    for (string request; std::getline(reader,request); ) {
        // ltrim
        request.erase(request.begin(), std::find_if(request.begin(), request.end(), [](int ch) {
            return !std::isspace(ch);
        }));
        if(!state->process_chat_request(*this, request)) {
            break;
        }
	}
	/*if(chat->strm.error || (state & CHAT_QUIT) || (state & CHAT_SOFT_QUIT)) {
		syslog(LOG_INFO, "Client quited\n");
        close_handle();
		return -1;
	}*/

	return 0;
}

int chat_connection::on_client_data(int fd, int status) {
	if(INVALID_FD == fd) {
		return 0;
    }
	// assert(fd == chat->strm.fd);
	int count = recv(fd, recv_buffer.data(), recv_buffer.capacity(), 0);
	if(count == 0) {
		syslog(LOG_INFO, "Client disconnected");
        close_handle();
		return -1;
	}
	if(count == -1) {
		syslog(LOG_ERR, "Error reading chat data %s", strerror(errno));
        close_handle();
		return -1;
	}
	if(count >= NGINZ_MAX_CHAT_MSG_SIZE) {
		syslog(LOG_INFO, "Disconnecting client for too big data input");
        close_handle();
		return -1;
	}
    recv_buffer.set_rd_length(count);
	int ret = on_client_data_helper();
	if(INVALID_FD == fd) {
        return -1;
	}
	return ret;
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

/*static struct chat_connection*chat_get(int token) {
	struct chat_connection*chat = opp_get(&chat_factory, token);
	if(chat->state & (CHAT_QUIT | CHAT_SOFT_QUIT | CHAT_ZOMBIE))
		return NULL;
	return chat;
}*/

chat_connection::chat_connection(
    int fd
    , event_loop& eloop
    , connection_state*state
    )
    : fd(fd)
    , eloop(eloop)
    , login_name("anonymous")
    , state(state)
    {
    if(INVALID_FD == fd || 0 == fd) {
        throw std::range_error("Invalid fild descriptor");
    }
    std::function<int(int,int)> data_callback = std::bind(&chat_connection::on_client_data, this, _1, _2);
    eloop.register_fd(fd, std::move(data_callback), NGINZ_POLL_ALL_FLAGS);
	// chat_zombie_module_init();
}

chat_connection::~chat_connection() {
    close_handle();
	//chat_zombie_module_deinit();
	//composite_unplug_bridge(chat_plugin_manager_get(), 0, chat_factory_show);
	//pm_unplug_callback(0, chat_factory_on_softquit);
}

