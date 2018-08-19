#ifndef NGINCC_PARALLEL_PIPELINE_HXX
#define NGINCC_PARALLEL_PIPELINE_HXX

#include <vector>
#include <initializer_list>
#include <sstream>

#include "ngincc_config.h"
#include "module.hxx"
#include "binary_coder.hxx"

namespace ngincc {
    namespace core {
        // forward declaration
        class plugin_manager;
        class event_loop;
        namespace parallel {
            class pipeline : public module {
            public:
                //! TODO make is singleton
                pipeline(ngincc::core::plugin_manager& core_plug,ngincc::core::event_loop& eloop);
                ~pipeline();
                //! Do not allow copy constructor (keeps the sockets open and avoids reopen)
                pipeline(const pipeline&) = delete;
                //! Do not allow assignment (keeps the sockets open and avoids reopen)
                pipeline& operator=(const pipeline&) = delete;
                //! TODO disable move as well
                //! \brief It sends message to a destination process.
                //! It sends broadcast message if the dnid is 0.
                //! \param dnid is the destination process id
                //! \param pkt is the message
                int pp_send(int dnid, uint8_t *msg, unsigned int msg_len) const;
                int pp_next_nid() const;
                int pp_next_worker_nid(int nid) const;
                //! \brief It checks if the process is master process.
                bool is_master() const;
                //! \brief It gets the raw socket to send data to the process.
                //! \returns -1 on failure, otherwise if returns the raw socket file descriptor.
                int pp_get_raw_fd(int nid) const;
                //! \brief rpc support
                int async_reply_worker(uint32_t destpid, uint32_t reply_token, const std::string& hook_space, uint32_t success, const std::initializer_list<std::reference_wrapper<const std::string> >&& more);
                //! \brief rpc support
                int async_request_master(uint32_t cb_token, const std::string &worker_hook, const std::string &master_hook, const std::initializer_list<std::reference_wrapper<const std::string> >&& more);
                //! \brief rpc support
                int async_request(uint32_t destpid, uint32_t cb_token, const std::string &worker_hook, const std::string &master_hook, const std::initializer_list<std::reference_wrapper<const std::string> >&& more);
            private:
                //! \brief It contains the node structure and pid
                struct ngincc_node {
                    int nid;
                    int fd[2]; //!< rule of thumb, write in fd[0], read from fd[1]
                    int raw_fd[2]; //!< this fd is for raw messaging(normally used to send socket fd)
                    //! TODO add shared-memory-queue here
                    ngincc_node();
                };
                enum {
                    MAX_PROCESS_COUNT = NGINZ_NUMBER_OF_PROCESSORS+1
                };
                pid_t masterpid;
                struct ngincc_node nodes[MAX_PROCESS_COUNT];
                struct ngincc_node*mynode;
                event_loop&   eloop;
                plugin_manager&   core_plug;
                ngincc::core::buffer_coder  recv_buffer; // NGINZ_MAX_BINARY_MSG_LEN
                ngincc::core::buffer_coder  send_buffer; // NGINZ_MAX_BINARY_MSG_LEN
                const struct ngincc_node*pp_find_node(int nid) const;
                int pp_simple_sendmsg(int through, uint8_t*msg, unsigned int msg_len) const;
                int pp_fork_parent_after_callback(int child_pid);
                int pp_fork_child_after_callback(int child_pid);
                int pp_update_siblings_pid(buffer_coder& msgbuf);
                int on_bubbles(int fd, int revents);
                int pp_simple_recvmsg_helper(int fd);
                int ping_command(std::vector<std::string> &command_args, std::ostringstream &output);
            };
        }
    }
}

#endif // NGINCC_PARALLEL_PIPELINE_HXX
