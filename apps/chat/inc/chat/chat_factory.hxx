#ifndef NGINZ_CHAT_CHAT_FACTORY_HXX
#define NGINZ_CHAT_CHAT_FACTORY_HXX

#include <string>
#include <vector>
#include <memory>

#include "chat/chat_connection_state.hxx"

namespace ngincc {
    namespace core {
        // forward declaration
        class event_loop;
    }
    namespace net {
        // forward declaration
        class raw_pipeline;
    }
    namespace db {
        class async_db;
    }
    namespace apps {
        namespace chat {
            class broadcast_room_module;
            class chat_factory {
            public:
                chat_factory(
                    ngincc::core::plugin_manager& core_plug
                    , ngincc::core::plugin_manager &chat_plug
                    , ngincc::core::event_loop& eloop
                    , ngincc::net::raw_pipeline& raw_pipe
                    , ngincc::db::async_db& adb_client
                    , broadcast_room_module& bcast_module
                );
                ~chat_factory();

                //! \addgroup chat connection list
                //! {@ 
                std::shared_ptr<chat_connection>& create_chat_connection(int fd, connection_state*state);
                std::shared_ptr<chat_connection>& get_chat(long long token);
                bool has_chat(long long token);
                int close_all();
                //! @}

                connection_state*get_connected();
                connection_state*get_logged_in();
                connection_state*get_in_room();
                connection_state*get_quitting();
            private:
                ngincc::core::event_loop &eloop;
                ngincc::net::raw_pipeline &raw_pipe;
                ngincc::db::async_db& adb_client;
                std::vector<std::shared_ptr<chat_connection> > clients;
                connection_state_connected connected_state;
                connection_state_logged_in logged_in_state;
                connection_state_in_room in_room_state;
                connection_state_quitting quitting_state;
            };
        }
    }
} 

#endif // NGINZ_CHAT_CHAT_FACTORY_HXX
