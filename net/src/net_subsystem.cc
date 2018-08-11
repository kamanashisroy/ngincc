
#include "module.hxx"
#include "plugin_manager.hxx"
#include "event_loop.hxx"
#include "base_subsystem.hxx"
#include "net_subsystem.hxx"
#include "tcp_listener_server.hxx"

using ngincc::core::plugin_manager;
using ngincc::core::event_loop;
using ngincc::net::tcp_listener_server;
using ngincc::net::net_subsystem;
using ngincc::net::raw_pipeline;


net_subsystem::net_subsystem()
    :base_subsystem()
    ,tcp_server_list{}
    ,tcp_listener(base_plug,base_event_loop,tcp_server_list)
    ,raw_pipe(base_plug,base_event_loop,base_pipe,tcp_server_list) {

}

net_subsystem::~net_subsystem() {
}


