#ifndef NGINZ_WORKER_HXX
#define NGINZ_WORKER_HXX

#include <functional>

#include "ngincc_config.h"
#include "module.hxx"

namespace ngincc {
    namespace core {
        // forward declaration
        class plugin_manager;
        class worker : public module {
        public:
            enum fiber_status {
                FIBER_STATUS_EMPTY = 0, // fiber is empty
                FIBER_STATUS_CREATED, // fiber is just created
                FIBER_STATUS_ACTIVATED, // fiber is activated/stepping
                FIBER_STATUS_DEACTIVATED, // fiber is deactivated/(not stepping)
                FIBER_STATUS_DESTROYED
            };

            #define register_fiber(x) register_full(x, __FILE__, __LINE__)
            //! $@{
            //! It unregisters a fiber from execution line.
            //! \param The fiber() callback function which is called in rotation 
            //! \return -1 on error, valid-id otherwise
            int register_full(const std::function<int(int)> &fiber, const char*filename, int lineno);
            int unregister(int fiber_id);
            //! $@}

            //! TODO make derived calls for fiber in the main.cc
            //! \brief It will dispatch all the fibers iteratively until cancelled
            int run(); // should only be called by main
            int quit(); // should only be called by main

            //! \brief make worker with plugin-manager
            worker(ngincc::core::plugin_manager &pm);
            worker() = delete; //!< no default constructor
        private:
            //! \brief It will dispatch all the fibers once
            int step();
            struct internal_fiber {
                int status;
                std::function<int(int)> fiber;
                const char*filename;
                int lineno;
            };
            volatile int internal_quit;
            struct internal_fiber fibers[NGINCC_MAX_FIBERS];
        };
    }
}

#endif // NGINZ_WORKER_HXX
