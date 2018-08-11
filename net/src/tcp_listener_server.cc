
#include <functional>
#include <vector>
#include <iostream>
#include <sstream>
#include <cstring> // defines strerror
#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>


#include "log.hxx"
#include "module.hxx"
#include "plugin_manager.hxx"
#include "event_loop.hxx"
#include "tcp_listener_server.hxx"

using std::string;
using std::ostringstream;
using std::endl;
using std::vector;
using std::function;
using ngincc::core::plugin_manager;
using ngincc::core::event_loop;
using ngincc::net::tcp_listener_server;
using ngincc::net::server_stack;
using namespace std::placeholders;
using namespace ngincc::net;

int tcp_listener_server::on_connect(int fd, int status, int server_index) {
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(struct sockaddr_in);
	int client_fd = accept(fd, (struct sockaddr *) &client_addr, &client_addr_len);
	if(client_fd < 0) {
		syslog(LOG_ERR, "Accept failed:%s\n", strerror(errno));
		eloop.unregister_fd(fd);
        tcp_server_list[server_index].on_server_error();
		close(fd);
		close(client_fd);
		return -1;
	}
    tcp_server_list[server_index].on_tcp_connection(client_fd);
	return 0;
}

int tcp_listener_server::stop(std::vector<std::string>& cmd_args, std::ostringstream& output) {

    // shutdown all the servers
    for(std::size_t i = 0; i < tcp_server_list.size(); i++) {
        if(-1 == tcp_server_fds[i]) {
            continue;
        }

        // unregister from event loop
		eloop.unregister_fd(tcp_server_fds[i]);

        // notify stack
        tcp_server_list[i].on_server_close();

        // close fd
		close(tcp_server_fds[i]);
		tcp_server_fds[i] = -1;
	}
	return 0;
}

int tcp_listener_server::start(std::vector<std::string>& cmd_args, std::ostringstream& output) {
    // iterate through all the available server-stack
    net_plugs.plug_call<vector<server_stack>&>("net/tcp/server",{tcp_server_list});

    for(std::size_t i = 0; i < tcp_server_list.size(); ++i) {
        auto&& stack = tcp_server_list[i];
        int tcp_port = stack.get_port();
        tcp_server_fds.push_back(-1);

        int server_fd = 0;
        // create socket
		if((server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
			syslog(LOG_ERR, "Failed to create socket:%s\n", strerror(errno));
            stack.on_server_error();
			continue;
		}

        // bind
		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		inet_aton("0.0.0.0", &(addr.sin_addr));
		addr.sin_port = htons(tcp_port);
		int sock_flag = 1;
		setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&sock_flag, sizeof(sock_flag));
		if(bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
			syslog(LOG_ERR, "tcp_listener,c Failed to bind port %d:%s\n", tcp_port, strerror(errno));
			close(server_fd);
            stack.on_server_error();
			continue;
		}
		if(listen(server_fd, NGINZ_TCP_LISTENER_BACKLOG) < 0) {
			syslog(LOG_ERR, "Failed to listen:%s\n", strerror(errno));
			close(server_fd);
            stack.on_server_error();
			continue;
		}

        // notify stack
        tcp_server_fds[i] = server_fd;
        stack.set_server_fd(server_fd);
        output << "tcp_listener.c:listening to port " << tcp_port << endl;
		syslog(LOG_INFO, "tcp_listener.c: listening to port %d\n", tcp_port);

        // register the server-fd in the event-loop
        function<int(int,int)> callback = std::bind(&tcp_listener_server::on_connect, this, _1, _2, i);
		eloop.register_fd(server_fd, move(callback), NGINZ_POLL_LISTEN_FLAGS);
	}
	return 0;
}

tcp_listener_server::tcp_listener_server(
        plugin_manager& net_plugs
        ,event_loop& eloop
        ,vector<server_stack>& tcp_server_list
    )
    : net_plugs(net_plugs)
    , eloop(eloop)
    , tcp_server_list(tcp_server_list)
    , tcp_server_fds{}
    {
	//if(!is_master()) // we only start it in the master
	//	return;
	//tcp_listener_start();
    function<int(vector<string>&,ostringstream&)> callback = 
        std::bind(&tcp_listener_server::start,this,_1,_2);
    net_plugs.plug_add("master/init", "It starts the tcp listener.", move(callback));
    callback = std::bind(&tcp_listener_server::stop,this,_1,_2);
    net_plugs.plug_add("fork/child/after", "Stop tcp listener on child.", move(callback));
    net_plugs.plug_add("shake/softquitall", "Stop tcp listener on child.", move(callback));
}

tcp_listener_server::~tcp_listener_server() {
	//TODO tcp_listener_stop(NULL, NULL);
	//TODO pm_unplug_callback(0, tcp_listener_stop);
	//TODO pm_unplug_callback(0, tcp_listener_stop);
}

