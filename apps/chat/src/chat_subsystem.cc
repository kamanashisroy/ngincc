#include <memory>
#include <functional>

#include "ngincc_config.h"
#include "event_loop.hxx"
#include "log.hxx"
#include "plugin_manager.hxx"
#include "chat/chat_factory.hxx"
#include "server_stack.hxx"
#include "chat_subsystem.hxx"

using std::vector;
using ngincc::core::plugin_manager;
using ngincc::core::event_loop;
using ngincc::net::server_stack;
using namespace ngincc::apps::chat;

chat_subsystem::chat_subsystem()
    : adb_master(base_plug, base_pipe)
    , adb_client(base_pipe)
    , factory(base_plug, chat_plug, base_event_loop, raw_pipe, adb_client)
    , chat_stack(base_plug, base_event_loop, base_pipe, raw_pipe, chat_plug, factory) {
    tcp_server_list.push_back(chat_stack);
}


chat_subsystem::~chat_subsystem() {
    // TODO Unregister hooks
	// chat_plugin_manager_module_deinit();
}

