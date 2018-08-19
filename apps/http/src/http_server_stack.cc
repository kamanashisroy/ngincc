
#include <memory>
#include <sstream>
#include <vector>
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
using namespace std::placeholders;

#define HTTP_WELCOME "http/welcome"
int http_server_stack::on_tcp_connection(int fd) {
    static const string command(HTTP_WELCOME);
	raw_pipe.send_socket(lb.next(), fd, get_port(), command);
	return 0;
}

#ifdef NGINZ_EVENT_DEBUG
static int on_http_debug(int fd, const void*cb_data) {
	struct http_connection*http = (struct http_connection*)cb_data;
	aroop_assert(http->strm.fd == fd);
	return 0;
}
#endif

int http_server_stack::on_server_error() {
    // WHAT TO DO ??
    return 0;
}

int http_server_stack::on_server_close() {
    // WHAT TO DO ??
    return 0;
}

int http_server_stack::set_server_fd(int server_fd) {
    // WHAT TO DO ??
    return 0;
}

int http_server_stack::on_connection_bubble(int fd, const string& command) {
	if(is_quiting)
		return 0;

    if(command.size() == 0) {
        syslog(LOG_ERR, "Possible BUG , cannot handle request %s", command.c_str());
        close(fd);
        return -1;
    }

    // TODO use plugin here to find the correct http-connection
	std::unique_ptr<http_connection> http(new default_http_connection(fd,eloop));
	if(!http) {
		syslog(LOG_ERR, "Could not create http object\n");
		close(fd);
		return -1;
	}

    // register it in the event loop
    clients.push_front(std::move(http));
    // TODO execute the command
	return 0;
}

int http_server_stack::get_port() const {
    return NGINZ_HTTP_PORT;
}

http_server_stack::http_server_stack(
    plugin_manager& net_plug // unused
    ,event_loop& eloop
    ,pipeline& base_pipe
    ,raw_pipeline& raw_pipe
) : eloop(eloop)
    , raw_pipe(raw_pipe)
    , is_quiting(false)
    , lb(base_pipe) {
    // TODO softquit all http servers net_plug.plug_add("shake/softquitall");
}

http_server_stack::~http_server_stack() {
	// TODO pm_unplug_bridge(0, http_accept_hookup);
}


