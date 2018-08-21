#ifndef NGINZ_NET_CHANNEL_HXX
#define NGINZ_NET_CHANNEL_HXX

#include <memory>
#include "binary_coder.hxx"

enum {
	INVALID_FD = -1,
};

/**
 * This is streaming/pipeline contruct. It allows the piping for filtering, tunneling or other purposes.
 * It is possible to implement proxy, delegator, bridge pattern using this contruct. 
 */

namespace ngincc {
    namespace core {
        class event_loop;
    }
    namespace net {

        class raw_pipeline;
        //! Net channel allows decoupling the data-read write part
        //! TODO use buffer coder instead of string
        class net_channel {
        public:
            virtual ~net_channel() = default;
            //! \brief On data receive ..
            virtual int on_recv(const std::string& content) = 0;
            virtual int on_recv(std::string&& content) = 0;
            /**
             * It sends/writes data to stream.
             * @param flags It is generally the flags available for send application namely, MSG_MORE, MSG_DONTWAIT ..
             */
            virtual int net_send(const std::string& content, int flags) = 0;
            virtual int net_send(std::string&& content, int flags) = 0;
            virtual int net_send_nonblock(const std::string& content, int flag) = 0;
            virtual int net_send_nonblock(std::string&& content, int flag) = 0;

            //! \brief terminate the connection and release the resource
            //! It closes the stream and removes the fd(if available) from the event loop. It disconnects itself(possibly destroying) from the next stream.
            //!
            virtual int close_handle(bool soft) = 0;
            /**
             * It transfers a file descriptor to another processor
             */
            virtual int transfer_parallel(int destpid, int proto_port, std::string &rpc_space) = 0;
        };

        //! \brief default_net_channel implements the network send/recv and allows
        //!        application-specific-extension by implementing the `on_recv` member function.
        class default_net_channel : public net_channel {
        public:
            default_net_channel() = delete;
            default_net_channel(int fd, ngincc::core::event_loop& eloop, ngincc::net::raw_pipeline& raw_pipe);
            virtual ~default_net_channel() override;
            virtual int net_send(const std::string& content, int flags) override;
            virtual int net_send(std::string&& content, int flags) override;
            virtual int net_send_nonblock(const std::string& content, int flag) override;
            virtual int net_send_nonblock(std::string&& content, int flag) override;
            virtual int close_handle(bool soft=true) override;
            virtual int transfer_parallel(int destpid, int proto_port, std::string &rpc_space) override;
        private:
            //! The fd of the stream. If there is no fd then it is set to -1/INVALID_FD.
            int fd;
            ngincc::core::event_loop& eloop;
            ngincc::net::raw_pipeline& raw_pipe;
            //! nonblocking send buffer
            //ngincc::core::buffer_coder send_buffer;
            //! incoming data buffer
            ngincc::core::buffer_coder recv_buffer;
            //! error state
            int error;
            int on_client_data(int fd, int status);
            int on_client_data_helper();
        };
    }
}

#define IS_VALID_STREAM(x) ((x)->fd != INVALID_FD || (x)->bubble_up != NULL)

#endif // NGINZ_NET_CHANNEL_HXX
