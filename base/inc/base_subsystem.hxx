#ifndef NGINZ_CORE_SUBSYSTEM_HXX
#define NGINZ_CORE_SUBSYSTEM_HXX

#include "ngincc_config.h"
#include "module.hxx"
#include "plugin_manager.hxx"
#include "worker.hxx"
#include "event_loop.hxx"
#include "lazy_worker.hxx"
#include "parallel/pipeline.hxx"
#include "shake/shake_intrprtr.hxx"

namespace ngincc {
    namespace core {
        class base_subsystem {
        public:
            //! \@{ getters
            worker& get_worker();
            //! Get the base plugin-manager
            plugin_manager& get_plugin_manager();
            //! \@}

            virtual int parallel_init();
            base_subsystem();
            virtual ~base_subsystem();
            int run();
        protected:
            plugin_manager base_plug;
            worker base_worker;
            lazy_worker base_lazy;
            event_loop_poll base_event_loop;
            parallel::pipeline base_pipe;
            shake::shake_intrprtr base_shake;
            //! Initialize master after the fork is done
            int master_init();
            //! Fork process
            int fork_process(int nclients);
            int rehash();
        };
    }
}

#endif // NGINZ_CORE_SUBSYSTEM_HXX
