
#include <any>
#include <functional>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <unistd.h>

#include "ngincc_config.h"
#include "log.hxx"
#include "event_loop.hxx"
#include "plugin_manager.hxx"
#include "binary_coder.hxx"
#include "parallel/pipeline.hxx"
#include "raw_pipeline.hxx"
#include "load_balancer.hxx"
#include "http/http_server_stack.hxx"
#include "http/http_connection.hxx"

using std::string;
using std::ostringstream;
using std::endl;
using std::vector;
using ngincc::core::plugin_manager;
using ngincc::core::event_loop;
using ngincc::core::parallel::pipeline;
using ngincc::net::raw_pipeline;
using ngincc::net::server_stack;
using ngincc::core::buffer_coder;
using ngincc::core::binary_coder;
using namespace ngincc::apps::http;


static struct http_hooks*hooks = NULL;
static int http_response_test_and_close(struct http_connection*http) {
	aroop_txt_t test = {};
	aroop_txt_embeded_set_static_string(&test, "HTTP/1.0 200 OK\r\nContent-Length: 9\r\n\r\nIt Works!\r\n");
	http->state = HTTP_QUIT;
	default_streamio_send(&http->strm, &test, 0);
	http->strm.close(&http->strm);
	return -1;
}

static int http_url_go(struct http_connection*http, aroop_txt_t*target) {
	int ret = 0;
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
	return ret;
}

int default_http_connection::http_url_parse(string& target_url) {
	aroop_txt_zero_terminate(user_data);
	aroop_assert(aroop_txt_is_zero_terminated(user_data));
	char*content = aroop_txt_to_string(user_data);
	char*prev_header = NULL;
	char*header = NULL;
	char*url = NULL;
	char*header_str = NULL;
	int header_len = 0;
	int skip_len = 0;
	int ret = 0;
	//aroop_txt_destroy(&http->content);
	while((header = strchr(content, '\n'))) {
		// skip the new line
		header++;
		header_len = header-content;
		if(prev_header == NULL) {
			// it is the request string ..
			if(header_len > NGINZ_MAX_HTTP_HEADER_SIZE) { // too big header
				ret = -1;
				break;
			}
			aroop_txt_embeded_buffer(target_url, header_len);
			aroop_txt_concat_string_len(target_url, content, header_len);
			aroop_txt_zero_terminate(target_url);
			header_str = aroop_txt_to_string(target_url);
			url = strchr(header_str, ' '); // GET url HTTP/1.1
			if(!url) {
				ret = -1;
				break;
			}
			url++; // skip the space
			skip_len = url - header_str;
			aroop_txt_shift(target_url, skip_len);
			header_str = url;
			url = strchr(header_str, ' '); // url HTTP/1.1
			if(!url) {
				ret = -1;
				break;
			}
			skip_len = url - header_str;
			aroop_txt_truncate(target_url, skip_len);
			// break;
		} else if(header_len <= 4) { // if it is \r\n\r\n
			// the content starts here
			
			aroop_txt_embeded_rebuild_copy_shallow(&http->content, user_data);
			skip_len = header - aroop_txt_to_string(user_data);
			aroop_txt_shift(&http->content, skip_len);
			break;
		}
		prev_header = header;
		content = header;
	}
	
	if(ret == -1)aroop_txt_destroy(target_url);
	return ret;
}

int default_http_connection::on_client_data(int fd, int status) {
	// aroop_assert(strm.fd == fd);
	int count = recv(fd, recv_buffer.data(), recv_buffer.capacity(), 0);
	if(count == 0) {
		syslog(LOG_INFO, "Client disconnected\n");
		http->strm.close(&http->strm);
		return -1;
	}
	if(count >= NGINZ_MAX_HTTP_MSG_SIZE) {
		syslog(LOG_INFO, "Disconnecting HTTP client for too big data input\n");
		http->strm.close(&http->strm);
		return -1;
	}
    recv_buffer.set_rd_length(count);
	//return x.processPacket(pkt);
	aroop_txt_t url = {};
	int response = http_url_parse(http, &recv_buffer, &url);
	if(response == 0) {
		// notify the page
		response = http_url_go(http, &url);
	}
	// cleanup
	aroop_txt_destroy(&url);
	aroop_txt_destroy(&http->content);
	if(http->state & (HTTP_SOFT_QUIT | HTTP_QUIT)) {
		http->strm.close(&http->strm);
		OPPUNREF(http);
	}
	return response;
}

default_http_connection::close() {
    if(-1 != fd) {
        eloop.unregister_fd(fd);
        fd = -1;
    }
}

default_http_connection::default_http_connection(int fd,event_loop& eloop) : fd(fd) {
    if(-1 == fd || 0 == fd) {
        throw std::range_error("Invalid fild descriptor");
    }
    std::function<int(int,int)> data_callback = std::bind(&default_http_connection::on_client_data, this, _1, _2);
    eloop.register_fd(fd, data_callback, NGINZ_POLL_ALL_FLAGS);
#ifdef NGINZ_EVENT_DEBUG
	eloop.register_debug(fd, on_http_debug);
#endif
}

default_http_connection::~default_http_connection() {
    close();
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
