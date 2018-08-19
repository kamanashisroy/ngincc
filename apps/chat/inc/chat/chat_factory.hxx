#ifndef NGINZ_CHAT_CHAT_FACTORY_HXX
#define NGINZ_CHAT_CHAT_FACTORY_HXX

#include <string>
#include <vector>
#include <memory>

#include "chat/chat_connection_state.hxx"

namespace ngincc {
    namespace apps {
        namespace chat {
            class chat_factory {
            public:
                chat_factory(
                    ngincc::core::plugin_manager& core_plug
                    , ngincc::core::plugin_manager &chat_plug);
                ~chat_factory();

                //! \addgroup chat connection list
                //! {@ 
                int add_chat_connection(std::unique_ptr<chat_connection>& chat);
                std::unique_ptr<chat_connection>& get_chat(long long token);
                bool has_chat(long long token);
                int close_all();
                //! @}
                connection_state*get_connected();
                connection_state*get_logged_in();
                connection_state*get_in_room();
                connection_state*get_quitting();
            private:
                //! TODO instead of unique_ptr, use object-pool
                std::vector<std::unique_ptr<chat_connection> > clients;
                connection_state_connected connected_state;
                connection_state_logged_in logged_in_state;
                connection_state_in_room in_room_state;
                connection_state_quitting quitting_state;
            };
        }
    }
} 

#endif // NGINZ_CHAT_CHAT_FACTORY_HXX
