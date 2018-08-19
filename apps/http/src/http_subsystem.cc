#include <memory>
#include <functional>

#include "ngincc_config.h"
#include "event_loop.hxx"
#include "log.hxx"
#include "plugin_manager.hxx"
#include "server_stack.hxx"
#include "http_subsystem.hxx"

using std::vector;
using ngincc::core::plugin_manager;
using ngincc::core::event_loop;
using ngincc::net::server_stack;
using namespace ngincc::apps::http;

http_subsystem::http_subsystem()
    : http_stack(base_plug, base_event_loop, base_pipe, raw_pipe) {
    tcp_server_list.push_back(http_stack);
    // tcp_listener.add_server_stack(new http_server_stack(base_plug, base_event_loop, base_pipe, raw_pipe));

    /*std::function<int(vector<std::unique_ptr<server_stack> >&)> callback = [this] (vector<std::unique_ptr<server_stack> >& output) {
        output.emplace_back();
        return 0;
    };
    base_plug.plug_add<vector<std::unique_ptr<server_stack> >&>("net/tcp/server", "It copies the hooks for future use.",  std::move(callback));*/
}


http_subsystem::~http_subsystem() {
    // TODO Unregister hooks
}

