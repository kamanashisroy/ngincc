#ifndef NGINCC_TCP_LISTENER_SERVER_HXX
#define NGINCC_TCP_LISTENER_SERVER_HXX

#include <vector>
#include <sstream>

#include "module.hxx"
#include "server_stack.hxx"
#include "plugin_manager.hxx"
#include "event_loop.hxx"

namespace ngincc {
    namespace net {
        //! \brief tcp-listerner server binds to a specific port and waits
        //!        for clients. It also delegates the client-fd to related protocol
        //!        stack
        class tcp_listener_server {
        public:
            tcp_listener_server(
                ngincc::core::plugin_manager& net_plugs
                , ngincc::core::event_loop& eloop
                , std::vector<ngincc::net::server_stack>& tcp_server_list
            );
            ~tcp_listener_server();
        private:
            ngincc::core::plugin_manager& net_plugs;
            ngincc::core::event_loop& eloop;

            //! contains all the protocol-stack here
            std::vector<ngincc::net::server_stack>& tcp_server_list;
            //! contains low-level server-handle
            std::vector<int> tcp_server_fds;

            int on_connect(int fd, int status, int server_index);
            int stop(std::vector<std::string>& cmd_args, std::ostringstream& output);
            int start(std::vector<std::string>& cmd_args, std::ostringstream& output);
        };
    }
}

#endif // NGINCC_TCP_LISTENER_SERVER_HXX

