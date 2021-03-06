
#include <any>
#include <functional>
#include <sstream>
#include <stdexcept>
#include <unistd.h>
#include <sys/socket.h> // defines send,recv

#include "ngincc_config.h"
#include "log.hxx"
#include "event_loop.hxx"
#include "plugin_manager.hxx"
#include "binary_coder.hxx"
#include "load_balancer.hxx"
#include "http/http_server_stack.hxx"
#include "http/http_connection.hxx"

using std::string;
using std::ostringstream;
using std::istringstream;
using std::endl;
using std::istream;
using ngincc::core::plugin_manager;
using ngincc::core::event_loop;
using ngincc::core::buffer_coder;
using namespace ngincc::apps::http;
using namespace std::placeholders;

#define HTTP_DEBUG(...) syslog(__VA_ARGS__)
//#define HTTP_DEBUG(...)

http_connection::~http_connection() {
    // close_handle();
}

/*static int http_response_test_and_close(struct http_connection*http) {
	aroop_txt_t test = {};
	aroop_txt_embeded_set_static_string(&test, "HTTP/1.0 200 OK\r\nContent-Length: 9\r\n\r\nIt Works!\r\n");
	http->state = HTTP_QUIT;
	default_streamio_send(&http->strm, &test, 0);
	close();
	return -1;
}*/

#define IT_WORKS "HTTP/1.0 200 OK\r\nContent-Length: 9\r\n\r\nIt Works!\r\n"

int default_http_connection::http_url_go(const string& target) {
	/*int ret = 0;
	int len = aroop_txt_length(target);
	if(aroop_txt_is_empty(target) || (len == 1))
		return http_response_test_and_close(http);
	http->is_processed = 0;
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_stackbuffer(&plugin_space, len+32);
	aroop_txt_concat_string(&plugin_space, "http");
	if(aroop_txt_char_at(target, 0) != '/') {
		aroop_txt_concat_char(&plugin_space, '/');
	}
	aroop_txt_concat(&plugin_space, target);
	if(aroop_txt_char_at(target, 5) == '_') // we do not allow hidden command like http/_webchat_hiddenjoin
		return http_response_test_and_close(http);
	ret = composite_plugin_bridge_call(http_plugin_manager_get(), &plugin_space, HTTP_SIGNATURE, http);
	if(!http->is_processed) { // if not processed
		// say not found
		aroop_txt_t not_found = {};
		aroop_txt_embeded_set_static_string(&not_found, "HTTP/1.0 404 NOT FOUND\r\nContent-Length: 9\r\n\r\nNot Found");
		default_streamio_send(&http->strm, &not_found, 0);
	}
	return ret;*/
    
	HTTP_DEBUG(LOG_NOTICE, "default_http_connection::sending ...it works!");
    const int send_len = sizeof(IT_WORKS)-1;
    if(send(fd, IT_WORKS, send_len, 0) != send_len) {
        return -1;
    }
    return 0;
}

int default_http_connection::http_url_parse(string& target_url) {
    // XXX it does not work with fragmented request
    istringstream reader(recv_buffer);
	string prev_header;
	string header;
	int ret = -1;

    // TODO parse content-length and content
    for (string header; std::getline(reader,header); ) {
	    HTTP_DEBUG(LOG_NOTICE, "default_http_connection::parsing %s", header.c_str());
		// skip the new line(ltrim)
        /*header.erase(header.begin(), std::find_if(header.begin(), header.end(), [](int ch) {
            return !std::isspace(ch);
        }));*/
		if(prev_header.size() == 0) {
			// it is the request string ..
			if(header.size() > NGINZ_MAX_HTTP_HEADER_SIZE) { // too big header
				break;
			}
            istringstream request_header(header);
            char delim;
            int token_index = 0;
            // GET url HTTP/1.1
            for(string token;request_header >> token;request_header >> delim, token_index++) {
                switch(token_index) {
                    case 0: // GET/PUT
	                    HTTP_DEBUG(LOG_NOTICE, "default_http_connection::parsing GET:%s", token.c_str());
                    break;
                    case 1: // URL
	                    HTTP_DEBUG(LOG_NOTICE, "default_http_connection::parsing URL:%s", token.c_str());
                        target_url = token;
                        ret = 0;
                    break;
                    default:
                    break;
                }
            }
		} else if(header.size() <= 4) { // if it is \r\n\r\n
			// the content starts here
			// TODO parse content
			break;
		}
		prev_header = std::move(header);
	}
	
	return ret;
}

int default_http_connection::on_client_data(int fd, int status) {
	// assert(strm.fd == fd);
	int count = recv(fd, &recv_buffer[0], recv_buffer.capacity(), 0);
	HTTP_DEBUG(LOG_NOTICE, "default_http_connection::on_client_data:>>Received client data");
	if(count == 0) {
		HTTP_DEBUG(LOG_NOTICE, "Client disconnected\n");
		close_handle();
		return -1;
	}
	if(count >= NGINZ_MAX_HTTP_MSG_SIZE) {
		syslog(LOG_NOTICE, "Disconnecting HTTP client for too big data input\n");
		close_handle();
		return -1;
	}
    recv_buffer.resize(count);
	HTTP_DEBUG(LOG_NOTICE, "read %d bytes", count);
	//return x.processPacket(pkt);
	string url;
	int response = http_url_parse(url);
    if(url.size() == 0) {
        // could not parse the user url, may be we cannot parse the fragmented request
		syslog(LOG_NOTICE, "Disconnecting HTTP client, we could not parse the url\n");
        close_handle();
        return -1;
    }
	if(0 == response) {
		// notify the page
		response = http_url_go(url);
	}
	// cleanup
	if(state & (HTTP_SOFT_QUIT | HTTP_QUIT)) {
		close_handle();
	}
	return response;
}

int default_http_connection::close_handle() {
    if(-1 != fd) {
        eloop.unregister_fd(fd);
        close(fd);
        fd = -1;
    }
    return 0;
}

default_http_connection::default_http_connection(int fd,event_loop& eloop)
    : fd(fd)
    , state(HTTP_CONNECTED)
    , recv_buffer(NGINZ_MAX_HTTP_MSG_SIZE, '\0')
    , eloop(eloop) {
    if(-1 == fd || 0 == fd) {
        throw std::range_error("Invalid fild descriptor");
    }
    std::function<int(int,int)> data_callback = std::bind(&default_http_connection::on_client_data, this, _1, _2);
    eloop.register_fd(fd, std::move(data_callback), NGINZ_POLL_ALL_FLAGS);
#ifdef NGINZ_EVENT_DEBUG
	eloop.register_debug(fd, on_http_debug);
#endif
}

default_http_connection::~default_http_connection() {
    close_handle();
}

#if 0
static int default_http_close(struct streamio*strm) {
	if(strm->bubble_up) {
		return strm->bubble_up->close(strm->bubble_up);
	}
	if(strm->fd != INVALID_FD) {
		struct http_connection*http = (struct http_connection*)strm;
		event_loop_unregister_fd(strm->fd);
		if(!(http->state & HTTP_SOFT_QUIT))close(strm->fd);
		strm->fd = -1;
	}
	return 0;
}
#endif
