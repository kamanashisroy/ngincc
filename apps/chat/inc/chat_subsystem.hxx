#ifndef NGINCC_CHAT_SUBSYSTEM_HXX
#define NGINCC_CHAT_SUBSYSTEM_HXX

#include "chat/chat_server_stack.hxx"
#include "async_db_master.hxx"
#include "async_db.hxx"
#include "net_subsystem.hxx"

namespace ngincc {
    namespace apps {
        namespace chat {
            class chat_subsystem : public ngincc::net::net_subsystem {
            public:
                chat_subsystem();
                virtual ~chat_subsystem();
            protected:
                ngincc::apps::chat::chat_server_stack chat_stack;
                ngincc::core::plugin_manager chat_plug;
                ngincc::db::async_db_master adb_master;
                ngincc::db::async_db adb_client;
            };
        }
    }
}

#endif // NGINCC_CHAT_SUBSYSTEM_HXX
