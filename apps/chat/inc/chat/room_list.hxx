#ifndef NGINCC_CHAT_ROOM_HXX
#define NGINCC_CHAT_ROOM_HXX

#include <string>
#include <vector>

// project headers
//#include "chat/room_list_master.hxx"

namespace ngincc {
    namespace core {
        // forward declaration
        class plugin_manager;
        class buffer_coder;
        namespace parallel {
            class pipeline;
        }
    }
    namespace apps {
        namespace chat {
            // forward declarations
            class chat_connection;
            class chat_factory;

            //! \brief room-list sets up rooms in different processes and provide room-information.
            class room_list {
            public:
                room_list() = delete;
                room_list(
                    ngincc::core::plugin_manager& core_plug
                    , ngincc::core::plugin_manager& chat_plug
                    , ngincc::core::parallel::pipeline& core_pipe
                    , ngincc::apps::chat::chat_factory& factory
                );
                ~room_list();
                int set_user_count(std::string& target_room, uint32_t user_count, uint32_t increment);
                int get_pid(std::string& target_room, uint32_t token, std::string& reply_hook);
                static inline const std::string on_async_request {"on/async/room/call"};
                static inline const std::string on_async_reply {"on/async/room/reply"};
                static inline const std::string ROOM_PREFIX {"room/pid/"};
            private:
                ngincc::core::plugin_manager& core_plug;
                ngincc::core::plugin_manager& chat_plug;
                ngincc::core::parallel::pipeline& core_pipe;
                ngincc::apps::chat::chat_factory& factory;

                int lookup_rooms(std::vector<std::string>& cmd_args
                    , ngincc::apps::chat::chat_connection& chat);
                int on_room_list_retrieval(ngincc::core::buffer_coder& response);
            };
        }
    }
}

#endif // NGINCC_CHAT_ROOM_HXX
