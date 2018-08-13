#ifndef NGINCC_NET_HTTP_CONNECTION_HXX
#define NGINCC_NET_HTTP_CONNECTION_HXX

#include <string>

#include "binary_coder.hxx"

namespace ngincc {
    namespace core {
        // forward declaration
        class event_loop;
    }
    namespace apps {
        namespace http {
            class http_connection {
            public:
                virtual ~http_connection() = 0;
            };
            class default_http_connection : public http_connection {
            public:
                default_http_connection(int fd,ngincc::core::event_loop& eloop);
                virtual ~default_http_connection() override;
            private:
                int fd;
                ngincc::core::buffer_coder recv_buffer;
                int http_url_parse(std::string& target_url);
                int on_client_data(int fd, int status);
            };
        }
    }
}

#endif // NGINCC_NET_HTTP_CONNECTION_HXX
