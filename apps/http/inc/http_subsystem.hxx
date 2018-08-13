#ifndef NGINCC_HTTP_SUBSYSTEM_HXX
#define NGINCC_HTTP_SUBSYSTEM_HXX

#include "http/http_server_stack.hxx"
#include "net_subsystem.hxx"

namespace ngincc {
    namespace apps {
        namespace http {
            class http_subsystem : public ngincc::net::net_subsystem {
            public:
                http_subsystem();
                virtual ~http_subsystem();
            protected:
                ngincc::apps::http::http_server_stack stack;
            };
        }
    }
}

#endif // NGINCC_HTTP_SUBSYSTEM_HXX
