#ifndef NGINZ_NET_CHANNEL_HXX
#define NGINZ_NET_CHANNEL_HXX

#include <memory>

enum {
	INVALID_FD = -1,
};

/**
 * This is streaming/pipeline contruct. It allows the piping for filtering, tunneling or other purposes.
 * It is possible to implement proxy, delegator, bridge pattern using this contruct. 
 */

namespace ngincc {
    namespace net {

        //! Net channel allows decoupling the data-read write part
        //! TODO use buffer coder instead of string
        class net_channel {
            virtual ~net_channel() = default;
            //! \brief On data receive ..
            virtual int on_recv(const std::string& content) = 0;
            virtual int on_recv(std::string&& content) = 0;
            /**
             * It sends/writes data to stream.
             * @param flags It is generally the flags available for send application namely, MSG_MORE, MSG_DONTWAIT ..
             */
            virtual int send(const std::string& content, int flags) = 0;
            virtual int send(std::string&& content, int flags) = 0;
            virtual int send_nonblock(const std::string& content, int flag) = 0;
            virtual int send_nonblock(std::string&& content, int flag) = 0;

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
            default_net_channel();
            virtual ~default_net_channel() override;
            virtual int send(const std::string& content, int flags) override;
            virtual int send(std::string&& content, int flags) override;
            virtual int send_nonblock(const std::string& content, int flag) override;
            virtual int send_nonblock(std::string&& content, int flag) override;
            virtual int close_handle(bool soft=true) override;
            virtual int transfer_parallel(int destpid, int proto_port, std::string &rpc_space) override;
        private:
            //! The fd of the stream. If there is no fd then it is set to -1/INVALID_FD.
            int fd;
            //! nonblocking send buffer
            std::string send_buffer;
            //! error state
            int error;
        };
    }
}

#define IS_VALID_STREAM(x) ((x)->fd != INVALID_FD || (x)->bubble_up != NULL)

#endif // NGINZ_NET_CHANNEL_HXX
