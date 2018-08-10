#ifndef NGINZ_SHAKE_INTERNAL_HXX
#define NGINZ_SHAKE_INTERNAL_HXX

#include "module.hxx"
#include "shake/quitall_internal.hxx"
#include "shake/test_internal.hxx"
#include "shake/help_internal.hxx"
#include "shake/enumerate_internal.hxx"

namespace ngincc {
    namespace core {
        // forward declaration
        class plugin_manager;
        class event_loop;
        class worker;
        namespace parallel {
            class pipeline;
        }
        namespace shake {
            class shake_intrprtr : public module {
            public:
                //! \brief create a shake-interpretter
                shake_intrprtr(ngincc::core::plugin_manager &shakeplugs, ngincc::core::worker &core_worker, ngincc::core::event_loop& eloop, ngincc::core::parallel::pipeline &pipe);
                //! \brief destroy shake-interpretter
                ~shake_intrprtr();
            private:
                int internal_unix_socket; //!< unix socket
                ngincc::core::plugin_manager &shakeplugs;
                ngincc::core::event_loop &eloop;

                // commands
                ngincc::core::shake::quitall quitall_module;   //!< quit-all module
                ngincc::core::shake::help help_module;         //!< quit-all module
                ngincc::core::shake::test test_module;         //!< test-module
                ngincc::core::shake::enumerate enumerate_module; //!< test-module
                bool is_master;

                int process_client(int client_fd);
                int setup_socket(std::vector<std::string>&, std::ostringstream&);
            };
        }
    }
}

#endif // NGINZ_SHAKE_INTERNAL_HXX
