#ifndef NGINCC_CHAT_SUBSYSTEM_HXX
#define NGINCC_CHAT_SUBSYSTEM_HXX

#include "chat/chat_server_stack.hxx"
#include "async_db_master.hxx"
#include "async_db.hxx"
#include "net_subsystem.hxx"
#include "chat/chat_factory.hxx"
#include "chat/room_list.hxx"
#include "chat/room_list_master.hxx"
#include "chat/join_room.hxx"
#include "chat/quit.hxx"
#include "chat/help.hxx"

namespace ngincc {
    namespace apps {
        namespace chat {
            class chat_subsystem : public ngincc::net::net_subsystem {
            public:
                chat_subsystem();
                virtual ~chat_subsystem();
            protected:
                ngincc::db::async_db_master adb_master;
                ngincc::db::async_db adb_client;
                ngincc::core::plugin_manager chat_plug;
                ngincc::apps::chat::chat_factory factory;
                ngincc::apps::chat::chat_server_stack chat_stack;
                // chat room
                ngincc::apps::chat::room_list_master room_module_master;
                ngincc::apps::chat::room_list room_module;
                // chat room join,leave
                ngincc::apps::chat::join_room join_module;
                ngincc::apps::chat::quit quit_module;
                ngincc::apps::chat::help help_module;
            };
        }
    }
}

#endif // NGINCC_CHAT_SUBSYSTEM_HXX
