#ifndef NGINZ_SHAKE_QUITALL_HXX
#define NGINZ_SHAKE_QUITALL_HXX

#include "module.hxx"         // defines module
namespace ngincc {
    namespace core {
        // forward declaration
        class plugin_manager;
        class worker;
        namespace parallel {
            class pipeline;
        }
        namespace shake {
            class quitall : public module {
            public:
                quitall(ngincc::core::plugin_manager &shakeplugs, ngincc::core::worker &core_worker, ngincc::core::parallel::pipeline &pipe);
                ~quitall();
            private:
                ngincc::core::worker &core_worker;
                ngincc::core::parallel::pipeline &pipe;
            };
        }
    }
}

#endif // NGINZ_SHAKE_QUITALL_HXX
