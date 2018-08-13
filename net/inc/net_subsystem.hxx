#ifndef NGINCC_NET_SUBSYSTEM_HXX
#define NGINCC_NET_SUBSYSTEM_HXX

#include <memory>

#include "ngincc_config.h"
#include "module.hxx"
#include "base_subsystem.hxx"
//#include "plugin_manager.hxx"
//#include "worker.hxx"
//#include "event_loop.hxx"
//#include "lazy_worker.hxx"
//#include "parallel/pipeline.hxx"
#include "tcp_listener_server.hxx"
#include "raw_pipeline.hxx"

namespace ngincc {
    namespace net {
        class net_subsystem : public ngincc::core::base_subsystem {
        public:
            net_subsystem();
            virtual ~net_subsystem();
        protected:
            std::vector<std::unique_ptr<ngincc::net::server_stack>> tcp_server_list;
            ngincc::net::tcp_listener_server tcp_listener;
            ngincc::net::raw_pipeline raw_pipe;
        };
    }
}

#endif // NGINCC_NET_SUBSYSTEM_HXX
