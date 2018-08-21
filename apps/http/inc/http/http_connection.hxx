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
            enum http_state {
                HTTP_CONNECTED = 0,
                HTTP_QUIT = 1<<3,
                HTTP_SOFT_QUIT = 1<<4
            };
            class http_connection {
            public:
                virtual ~http_connection();
                virtual int close_handle() = 0;
            };

            class default_http_connection : public http_connection {
            public:
                default_http_connection(int fd,ngincc::core::event_loop& eloop);
                virtual ~default_http_connection() override;
                virtual int close_handle() override;
            private:
                int fd;
                int state;
                std::string recv_buffer;
                ngincc::core::event_loop& eloop;

                int http_url_parse(std::string& target_url);
                int http_url_go(const std::string& target_url);
                int on_client_data(int fd, int status);
            };
        }
    }
}

#endif // NGINCC_NET_HTTP_CONNECTION_HXX
