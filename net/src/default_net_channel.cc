
#include <istream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h> // defines send,recv

#include "ngincc_config.h"
#include "log.hxx"
#include "net_channel.hxx"
#include "raw_pipeline.hxx"

using std::string;
using std::stringstream;
using ngincc::core::buffer_coder;
using ngincc::core::event_loop;
using ngincc::net::raw_pipeline;
using ngincc::net::net_channel;
using ngincc::net::default_net_channel;
using namespace std::placeholders;

#define NET_DEBUG(...) syslog(__VA_ARGS__)
//#define NET_DEBUG(...)


int default_net_channel::net_send(string&& content, int flag) {
    NET_DEBUG(LOG_NOTICE, "Sending(forwarding) %s", content.c_str());
    return net_send(content, flag);
}

int default_net_channel::net_send(const string& content, int flag) {
	if(INVALID_FD == fd) {
		syslog(LOG_ERR, "There is a dead chat\n");
		return -1;
	}
    NET_DEBUG(LOG_NOTICE, "Sending %s", content.c_str());
	return send(fd, content.c_str(), content.size(), flag);
}

int default_net_channel::net_send_nonblock(string&& content, int flag) {
    return net_send_nonblock(content, flag);
}

int default_net_channel::net_send_nonblock(const string& content, int flag) {
	if(INVALID_FD == fd) {
		syslog(LOG_ERR, "There is a dead chat\n");
		return -1;
	}
	int err = send(fd, content.c_str(), content.size(), flag /*| MSG_DONTWAIT*/); // TODO use nonblocking io
	if(err == -1) {
		if((errno == EWOULDBLOCK || errno == EAGAIN)) {
			// TODO implement nonblocking io
			syslog(LOG_ERR, "Could not write network data asynchronously ..");
			error = errno;
			return -1;
		}
		syslog(LOG_ERR, "closing socket because while streaming :%s", strerror(errno));
		error = errno;
		//close_handle();
		return err;
	}
	return err;
}

int default_net_channel::close_handle(bool soft) {
	if(INVALID_FD != fd) {
		eloop.unregister_fd(fd);
		close(fd);
		fd = INVALID_FD;
	}
	return 0;
}

int default_net_channel::transfer_parallel(int destpid, int proto_port, string& command) {
	//syslog(LOG_NOTICE, "default_transfer_parallel: transfering to %d", destpid);
	//if(strm->bubble_up) {
		//syslog(LOG_NOTICE, "default_transfer_parallel: bubble up ");
	//	return strm->bubble_up->transfer_parallel(strm->bubble_up, destpid, proto_port, cmd);
	//}
	eloop.unregister_fd(fd);
	return raw_pipe.send_socket(destpid, fd, proto_port, command);
    // TODO close the fd here
}

int default_net_channel::on_client_data_helper() {
	if(recv_buffer.in_avail() == 0) { // sanity check
		return 0;
	}
    stringstream reader;
    //reader.set_rdbuf(recv_buffer.rdbuf());
    reader << recv_buffer.rdbuf();
    // NOTE we require newline at the end of data
    // TODO fix this to allow a stream or partial lines
    for (string request; std::getline(reader,request); ) {
    //for (string request; reader.getline(request);) {
        // ltrim
        request.erase(request.begin(), std::find_if(request.begin(), request.end(), [](int ch) {
            return !std::isspace(ch);
        }));
        if(on_recv(request)) {
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



int default_net_channel::on_client_data(int fd, int status) {
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
        // TODO handle error cases and try recover
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

default_net_channel::default_net_channel(int fd, event_loop& eloop, raw_pipeline& raw_pipe)
    : fd(fd)
    , eloop(eloop)
    , raw_pipe(raw_pipe)
    //, recv_buffer
    , error(0){
    NET_DEBUG(LOG_NOTICE, "new channel for %d", fd);
    std::function<int(int,int)> data_callback = std::bind(&default_net_channel::on_client_data, this, _1, _2);
    eloop.register_fd(fd, std::move(data_callback), NGINZ_POLL_ALL_FLAGS);
}

default_net_channel::~default_net_channel() {
    close_handle();
}


