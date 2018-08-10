#ifndef NGINZ_SHAKE_HELP_HXX
#define NGINZ_SHAKE_HELP_HXX

#include "module.hxx"         // defines module

namespace ngincc {
    namespace core {
        // forward declaration
        class plugin_manager;
        namespace shake {
            class help : public module {
            public:
                help(plugin_manager&pm);
                ~help();
            };
        }
    }
}

#endif // NGINZ_SHAKE_HELP_HXX
