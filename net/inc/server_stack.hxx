#ifndef NGINCC_SERVER_STACK_HXX
#define NGINCC_SERVER_STACK_HXX

#include <string>
#include "module.hxx"

namespace ngincc {
    namespace net {
        //! \brief protocol stack defines the callback when tcp-connection is created
        class server_stack {
        public:
            virtual ~server_stack();
            //! \brief when a tcp-client is accepted
            virtual int on_tcp_connection(int client_fd);
            //! \brief when a tcp-client is transferred to the (current)worker process
            virtual int on_connection_bubble(int fd);
            //! \brief get the port number to bind
            virtual int get_port() const;
            //! \brief set the server fd, it is good place to set io-flags
            virtual int set_server_fd(int server_fd);
            //! \brief notify the server is closed
            virtual int on_server_close();
            //! \brief notify the server-error
            virtual int on_server_error();
        };
    }
}

#endif // NGINCC_SERVER_STACK_HXX

