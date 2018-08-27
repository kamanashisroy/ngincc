#ifndef NGINCC_CHAT_JOIN_ROOM_HXX
#define NGINCC_CHAT_JOIN_ROOM_HXX

#include <memory>
#include <vector>
#include <string>

namespace ngincc {
    namespace core {
        // forward declaration
        class plugin_manager;
        class buffer_coder;
    }
    namespace db {
        // forward declaration
        class async_db;
    }
    namespace apps {
        namespace chat {

            // forward declaration
            class chat_connection;
            class chat_factory;
            class broadcast_room_module;
            //! Provides '/join <room-name>' command to allow chat-user to join room.
            //! \sa asyncchat
            class join_room {
            public:
                join_room(
                    ngincc::core::plugin_manager& core_plug
                    ,ngincc::core::plugin_manager& chat_plug
                    ,ngincc::db::async_db& adb_client
                    ,ngincc::apps::chat::chat_factory& factory
                    ,broadcast_room_module& bcast_module
                );
                ~join_room();
                
            private:
                ngincc::core::plugin_manager& core_plug;
                ngincc::core::plugin_manager& chat_plug;
                ngincc::db::async_db& adb_client;
                ngincc::apps::chat::chat_factory& factory;
                broadcast_room_module& bcast_module;

                //! \brief receive the process-id that contains chat-room
                int on_target_pid_retrieval(ngincc::core::buffer_coder& recv_buffer);
                //! \brief take action to allow user join the room
                int join_helper(std::shared_ptr<chat_connection>& chat, std::string& room, int pid);
                //! \brief respond to user `/join` command
                int process_join(std::vector<std::string>& cmd_args, chat_connection& chat);
            };
        }
    }
}


#endif // NGINCC_CHAT_JOIN_ROOM_HXX
