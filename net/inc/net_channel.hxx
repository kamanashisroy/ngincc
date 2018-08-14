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
        // TODO use buffer coder instead of string
        class net_channel {
            virtual ~net_channel() = 0;
            /**
             * On data receive ..
             */
            virtual int on_recv(string &content) = 0;
            /**
             * It sends/writes data to stream.
             * @param flags It is generally the flags available for send application namely, MSG_MORE, MSG_DONTWAIT ..
             */
            virtual int send(string &content, int flags) = 0;
            virtual int send_nonblock(string& content, int flag) = 0;
            /**
             * It closes the stream and removes the fd(if available) from the event loop. It disconnects itself(possibly destroying) from the next stream.
             */
            virtual int close_handle() = 0;
            /**
             * It transfers a file descriptor to another processor
             */
            virtual int transfer_parallel(int destpid, int proto_port, string &rpc_space) = 0;
        };
        class default_net_channel : public net_channel {
        public:
            default_net_channel();
            virtual ~default_net_channel() override;
            virtual int on_recv(string &content) override;
            virtual int send(string &content, int flags) override;
            virtual int send_nonblock(string& content, int flag) override;
            virtual int close_handle() override;
            virtual int transfer_parallel(int destpid, int proto_port, string &rpc_space) override;
        private:
            /**
             * The fd of the stream. If there is no fd then it is set to -1/INVALID_FD.
             */
            int fd;
            /**
             * nonblocking send buffer
             */
            string send_buffer;
            int error;
            /**
             * It is the next aggregated stream. In one way cases, it is garbage collected. It needs not be garbage collected in two way situation(avoid circular reference). It facilitates chain-of-responsibility pattern.
             */
            std::shared_ptr<net_channel> bubble_up;
            std::shared_ptr<net_channel> &bubble_down;
        };
    }
}

#define IS_VALID_STREAM(x) ((x)->fd != INVALID_FD || (x)->bubble_up != NULL)

#endif // NGINZ_NET_CHANNEL_HXX
