#ifndef EVENT_LOOP_HXX
#define EVENT_LOOP_HXX

#include <functional>

#include "module.hxx"
#include "ngincc_config.h"

namespace ngincc {
    namespace core {
        class event_loop : public module {
        public:
            virtual ~event_loop();
            virtual int register_fd(int fd //!< low-level file handle
                , std::function<int(int //!< the fd
                                ,int //!< received event
                                )>&& callback 
                , int requested_events //! < requested event flag(see poll.h)
            ) = 0;
            virtual int unregister_fd(int fd) = 0;
            virtual int count() const = 0;
        };
        // forward declaration
        class plugin_manager;
        class worker;
        class event_loop_poll : public event_loop {
        public:
            event_loop_poll(ngincc::core::worker &, ngincc::core::plugin_manager &);
            ~event_loop_poll();
            virtual int register_fd(int fd, std::function<int(int,int)>&& callback, int requested_events) override;
            virtual int unregister_fd(int fd) override;
            virtual int count() const override;
        private:
            struct event_callback {
                std::function<int(int,int)> on_event; // first argument is fd and second is returned_events
            #ifdef NGINZ_EVENT_DEBUG
                int (*on_debug)(int fd, const void*debug_data);
            #endif
            };
            int internal_nfds;
            struct pollfd internal_fds[MAX_POLL_FD];
            struct event_callback internal_callback[MAX_POLL_FD];
            int batch_unregister();
            int step_helper(int count);
            int test_helper(int count);
        };
    }
}

#endif // EVENT_LOOP_HXX
