#ifndef NGINCC_CHAT_ROOM_MASTER_HXX
#define NGINCC_CHAT_ROOM_MASTER_HXX

#include <string>

namespace ngincc {
    namespace core {
        class plugin_manager;
        class buffer_coder;
        namespace parallel {
            class pipeline;
        }
    }
    namespace db {
        class async_db_master;
        class async_db;
    }
    namespace apps {
        namespace chat {
            class broadcast_room_module;
            class room_list_master {
            public:
                room_list_master() = delete;
                room_list_master(
                    ngincc::core::plugin_manager& core_plug
                    , ngincc::db::async_db_master& adb_master
                    , ngincc::db::async_db& adb_client
                    , ngincc::core::parallel::pipeline& core_pipe
                    , broadcast_room_module& bcast_module
                    );
                ~room_list_master();
            private:
                unsigned int internal_child_count;
                ngincc::core::plugin_manager& core_plug;
                ngincc::db::async_db_master& adb_master;
                ngincc::db::async_db& adb_client;
                ngincc::core::parallel::pipeline& core_pipe;
                broadcast_room_module& bcast_module;

                int get_user_count(const std::string& xroom);
                int list_rooms_hook(ngincc::core::buffer_coder& recv_buffer);
                int master_init(std::vector<std::string>& , std::ostringstream& output);
                int parent_after_hook(int pid);
                int child_after_hook(int pid);
                int make_room(unsigned int index);
            };
        }
    }
}

#endif // NGINCC_CHAT_ROOM_MASTER_HXX
