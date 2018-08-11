#ifndef NGINZ_RAW_PIPELINE_HXX
#define NGINZ_RAW_PIPELINE_HXX


#include "module.hxx"
#include "plugin_manager.hxx"
#include "event_loop.hxx"
#include "parallel/pipeline.hxx"

namespace ngincc {
    namespace net {
        //! \brief raw-pipeline is used to send socket-fd to client processes
        class raw_pipeline {
        public:
            raw_pipeline(
                ngincc::core::plugin_manager& net_plugs
                , ngincc::core::event_loop& eloop
                , ngincc::core::parallel::pipeline& pipe
                , std::vector<ngincc::net::server_stack>& tcp_server_list
            );
            ~raw_pipeline();
            //! \brief send the socket-fd to destination process.
            //!        while the message is processed by the target_plugin_space
            //! \param   destpid        destination process id
            //! \param   socket         socket-fd
            //! \param   plugin_space   a plugin-space that handles the socket
            //!                         in destination process
            int send_socket(int destpid, int socket, std::string& plugin_space);
        private:
            ngincc::core::buffer_coder recv_buffer;
            ngincc::core::event_loop& eloop;
            ngincc::core::parallel::pipeline& pipe;
            std::vector<ngincc::net::server_stack>& tcp_server_list;

            int on_raw_socket_setup(int raw_fd);
            int on_raw_recv_socket(int fd, int events);
            int sendmsg_helper(int through, int target, std::string& cmd);
            int recvmsg_helper(int through, int&target);
        };
    }
}

#endif // NGINZ_RAW_PIPELINE_HXX
