
#include <memory>
#include <functional>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/socket.h> // defines send,recv

#include "ngincc_config.h"
#include "log.hxx"
#include "event_loop.hxx"
#include "plugin_manager.hxx"
#include "binary_coder.hxx"
#include "parallel/pipeline.hxx"
#include "raw_pipeline.hxx"
#include "load_balancer.hxx"
#include "chat/chat_server_stack.hxx"
#include "chat/chat_connection.hxx"
//#include "chat/chat_zombie.h"
//#include "chat/chat_plugin_manager.h"

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
using namespace ngincc::apps::chat;
using namespace std::placeholders;

#define CHAT_WELCOME "/_welcome"
int chat_server_stack::on_tcp_connection(int fd) {
    static const string command(CHAT_WELCOME);
	raw_pipe.send_socket(lb.next(), fd, get_port(), command);
	return 0;
}

int chat_server_stack::on_server_error() {
    on_server_close();
    return 0;
}

int chat_server_stack::on_server_close() {
    is_quiting = true;
    factory.close_all();
    // TODO close server
    return 0;
}

int chat_server_stack::get_port() const {
    return NGINZ_CHAT_PORT;
}

int chat_server_stack::set_server_fd(int server_fd) {
    // WHAT TO DO ??
    return 0;
}

int chat_server_stack::on_connection_bubble(int fd, const string& rpc_space) {
	if(is_quiting)
		return -1;
	/*int ret = 0;
	// create new connection
	std::unique_ptr<chat_connection> chat(new chat_connection(fd,eloop,raw_pipe,factory.get_connected()));
    if(!chat) {
		syslog(LOG_ERR, "Could not create chat object\n");
		close(fd);
		return -1;
    }

    factory.add_chat_connection(chat);
    factory.create_chat_connection(fd);*/

    
	auto& chat = factory.create_chat_connection(fd, factory.get_connected());
    if(!chat) {
		syslog(LOG_ERR, "Could not create chat object\n");
		close(fd);
		return -1;
    }
    chat->on_recv(rpc_space);
	return 0;
}

chat_server_stack::chat_server_stack(
    plugin_manager& net_plug // unused
    ,event_loop& eloop
    ,pipeline& base_pipe
    ,raw_pipeline& raw_pipe
    ,plugin_manager& chat_plug
    ) : eloop(eloop)
    , raw_pipe(raw_pipe)
    , is_quiting(false)
    , lb(base_pipe)
    , factory(net_plug,chat_plug,eloop,raw_pipe) {
	// aroop_txt_embeded_set_static_string(&cannot_process, "Cannot process the request\n");
	// aroop_txt_embeded_buffer(&recv_buffer, NGINZ_MAX_CHAT_MSG_SIZE);
	// protostack_set(NGINZ_CHAT_PORT, &chat_protostack);
    std::function<int(vector<string>&, ostringstream&)> quit_callback
        = [this] (vector<string>&,ostringstream& output) {
            output << "Quiting chat server" << endl;
            return on_server_close();
    };
    net_plug.plug_add("shake/softquitall", "It does not allow new connection.", std::move(quit_callback));
}

chat_server_stack::~chat_server_stack() {
	// protostack_set(NGINZ_HTTP_PORT, NULL);
	// TODO pm_unplug_callback(0, chat_accept_on_softquit);
    on_server_close();
}

