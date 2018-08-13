
#include <functional>
//#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include <cstdio>
#include <cstring> // defines memset
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <vector>

#include "ngincc_config.h"
#include "log.hxx"
#include "module.hxx"
#include "plugin_manager.hxx"
#include "worker.hxx"
#include "event_loop.hxx"
#include "shake/quitall_internal.hxx"
#include "shake/help_internal.hxx"
#include "shake/test_internal.hxx"
#include "shake/enumerate_internal.hxx"
#include "shake/shake_intrprtr.hxx"
//#include "parallel/pipeline.hxx"

using std::string;
using std::istringstream;
using std::ostringstream;
using std::vector;
using ngincc::core::event_loop;
using ngincc::core::worker;
using ngincc::core::plugin_manager;
using ngincc::core::parallel::pipeline;
using namespace ngincc::core::shake;
using namespace std::placeholders;

shake_intrprtr::shake_intrprtr(plugin_manager&shakeplugs,worker&core_worker,event_loop&eloop,pipeline&pipe)
    : internal_unix_socket(-1)
    , recv_string(128,'\0')
    , is_master(false)
    , shakeplugs(shakeplugs)
    , eloop(eloop)
    , quitall_module(shakeplugs,core_worker,pipe)
    , help_module(shakeplugs)
    , test_module(shakeplugs)
    , enumerate_module(shakeplugs) {

    std::function<int(vector<string>&,ostringstream&) > callback = std::bind(&shake_intrprtr::setup_socket,this,_1,_2);
    shakeplugs.plug_add("master/init", "Setup unix shake-socket", move(callback));
}

shake_intrprtr::~shake_intrprtr() {
	if(!is_master) {
		return;
	}
	//event_loop_unregister_fd(STDIN_FILENO);
	// TODO event_loop_unregister_fd(internal_unix_socket);
	if(internal_unix_socket != -1) {
		close(internal_unix_socket);
		internal_unix_socket = -1;
	}
	unlink(NGINCC_SHAKE_SOCK_FILE);
}

int shake_intrprtr::process_client(int fd) {
	// read data from stdin
    //recv_string.clear(); // NOTE it may change the string size
	int cmdlen = read(fd, &recv_string[0], recv_string.capacity()); // XXX it will block the total process ..
	if(cmdlen <= 0)
		return 0;

    // tokenize
    istringstream reader(recv_string);
    std::vector<string> command_args;
    for(string token;std::getline(reader,token,' ');) {
        if(token.size() == 0) {
            continue;
        }
        command_args.push_back(token);
    }

    // read the last token
    string last_token;
    reader >> last_token;
    if(last_token.size()) {
        command_args.push_back(last_token);
    }

    // sanity check
    if(0 == command_args.size()) {
        return 0;// skip it
    }

    // call plugin
    string plugin_space = string("shake/") + command_args[0];
    ostringstream writer;
    //std::tuple<std::vector<string>,stringstream > args({command_args,writer});
    std::tuple<std::vector<string>&,ostringstream& > args = std::make_tuple(std::ref(command_args),std::ref(writer));
    // shakeplugs.plug_call(plugin_space, std::make_tuple(command_args, writer));
    shakeplugs.plug_call<std::vector<string>&,ostringstream& >(std::move(plugin_space), std::move(args));
    string outstr = writer.str();
    [[maybe_unused]] int result = write(fd, outstr.c_str(), outstr.size());
	return 0;
}

int shake_intrprtr::setup_socket(vector<string>&, ostringstream& output) {
    is_master = true;
	if(internal_unix_socket != -1) {
        throw std::overflow_error("Cannot setup socket, the socket is not empty");
    }
	unlink(NGINCC_SHAKE_SOCK_FILE);
	if((internal_unix_socket = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		syslog(LOG_ERR, "Shake: failed to create unix socket:%s\n", strerror(errno));
		return -1;
	}
	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, NGINCC_SHAKE_SOCK_FILE, sizeof(addr.sun_path)-1);
	if(bind(internal_unix_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		syslog(LOG_ERR, "shake:Failed bind:%s\n", strerror(errno));
		close(internal_unix_socket);
		return -1;
	}
	if(listen(internal_unix_socket, 5) < 0) {
		syslog(LOG_ERR, "shake:Failed to listen:%s\n", strerror(errno));
		close(internal_unix_socket);
		return -1;
	}
	output << "listening to unix socket..." << std::endl;
	//event_loop_register_fd(STDIN_FILENO, on_shake_command, NULL, NGINZ_POLL_ALL_FLAGS);
    eloop.register_fd(internal_unix_socket, [this] (int fd, int revent) -> int {
            if(internal_unix_socket == -1)
                return 0;
            if(internal_unix_socket != fd) {
                throw std::invalid_argument("Shake: expecting unix-socket received other");
            }
            int client_fd = accept(internal_unix_socket, NULL, NULL);
            if(client_fd < 0) {
                syslog(LOG_ERR, "Shake: Accept failed:%s\n", strerror(errno));
                close(client_fd);
                return -1;
            }
            process_client(client_fd);
            close(client_fd);
            return 0;
        },NGINZ_POLL_ALL_FLAGS);
	return 0;
}


