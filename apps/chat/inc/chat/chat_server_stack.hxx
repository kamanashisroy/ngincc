#ifndef NGINCC_CHAT_SERVER_STACK_HXX
#define NGINCC_CHAT_SERVER_STACK_HXX

#include <memory>
#include <vector>
#include <string>

#include "module.hxx"
#include "server_stack.hxx"
#include "binary_coder.hxx"
#include "load_balancer.hxx"
#include "chat_connection_state.hxx"

namespace ngincc {
    namespace core {
        // forward declaration
        class event_loop;
        class plugin_manager;
        namespace parallel {
            class pipeline;
        }
    }
    namespace net {
        // forward declaration
        class raw_pipeline;
    }
    namespace apps {
        namespace chat {
            // forward declaration
            class chat_connection;
            class chat_factory;
            //! \brief chat-server creates chat-connection on tcp-client accept
            //!
            //! While the server listen in the master, the chat-connection
            //! is created in the child process.
            //!
            class chat_server_stack : public ngincc::net::server_stack {
            public:
                chat_server_stack() = delete;
                chat_server_stack(
                    ngincc::core::plugin_manager& core_plug
                    , ngincc::core::event_loop& eloop
                    , ngincc::core::parallel::pipeline& base_pipe
                    , ngincc::net::raw_pipeline& raw_pipe
                    , ngincc::core::plugin_manager& chat_plug
                    , chat_factory& factory
                );
                virtual ~chat_server_stack() override;
                //! \brief when a tcp-client is accepted
                virtual int on_tcp_connection(int client_fd) override;
                //! \brief when a tcp-client is transferred to the (current)worker process
                virtual int on_connection_bubble(int fd, const std::string& rpc_space) override;
                //! \brief get the port number to bind
                virtual int get_port() const override;
                //! \brief set the server fd, it is good place to set io-flags
                virtual int set_server_fd(int server_fd) override;
                //! \brief notify the server is closed
                virtual int on_server_close() override;
                //! \brief notify the server-error
                virtual int on_server_error() override;
            private:
                ngincc::core::event_loop &eloop;
                ngincc::net::raw_pipeline &raw_pipe;
                bool is_quiting;
                ngincc::core::buffer_coder command_buffer;
                ngincc::apps::round_robin_load_balancer lb;
                chat_factory& factory;
            };
        }
    }
}

#endif // NGINCC_CHAT_SERVER_STACK_HXX
