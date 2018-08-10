#ifndef NGINZ_SHAKE_ENUMERAT_HXX
#define NGINZ_SHAKE_ENUMERAT_HXX

#include "module.hxx"         // defines module

namespace ngincc {
    namespace core {
        // forward declaration
        class plugin_manager;
        namespace shake {
            class enumerate : public module {
            public:
                enumerate(plugin_manager&pm);
                ~enumerate();
            private:
                long long accumulator;
            };
        }
    }
}

#endif // NGINZ_SHAKE_ENUMERAT_HXX
