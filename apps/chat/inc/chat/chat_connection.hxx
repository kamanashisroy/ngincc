#ifndef NGINCC_APPS_CHAT_CONNECTION_HXX
#define NGINCC_APPS_CHAT_CONNECTION_HXX

#include <string>
#include <sstream>
#include <vector>

#include "net_channel.hxx"
#include "binary_coder.hxx"

namespace ngincc {
    namespace core {
        // forward declaration
        class event_loop;
        class plugin_manager;
        class buffer_coder;
    }
    namespace net {
        class raw_pipeline;
    }
    namespace apps {
        namespace chat {
            class connection_state;
            class chat_factory;
            //! \brief Holds the information about a client-connection
            class chat_connection : public ngincc::net::default_net_channel {
            public:
                chat_connection(
                    int fd                                      //!< connection handle
                    , long long token                           //!< works as unique-id
                    , ngincc::core::event_loop& eloop           //!< event-loop task
                    , ngincc::net::raw_pipeline& raw_pipe       //!< pipeline for socket
                    , connection_state*state
                );
                chat_connection() = delete;

                ~chat_connection();

                // void desc(std::ostringstream& output);
                //! \brief describe the connection in human-readable way
                const std::string to_string() const;

                inline long long get_token() const {
                    return token;
                }

                //! \brief It allowes state transision
                int set_state(connection_state*next_state);

                //! \brief get connection state
                bool in_state(const std::type_info& ) const;
                //connection_state*get_state();

                int set_login_name(const std::string& login_name);
                const std::string& get_login_name() const;

                //! \addtogroup net_channel
                //! @{
                virtual int on_recv(const std::string &content) override;
                virtual int on_recv(std::string&& content) override;
                //! @}
            private:

                int fd;                                         //!< connection handle
                int flags;                                      //!< connection flags
                long long token;                                //!< unique identifier of the connection
                ngincc::core::event_loop& eloop;
                std::string login_name;
                ngincc::apps::chat::connection_state* state;    //!< state of connection

                //! it is used for broadcast/setting the name
                int on_response_callback(std::string& command);
            };
        }
    }
}

#endif // NGINCC_APPS_CHAT_CONNECTION_HXX
