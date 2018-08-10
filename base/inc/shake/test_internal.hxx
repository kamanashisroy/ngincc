#ifndef NGINZ_SHAKE_TEST_HXX
#define NGINZ_SHAKE_TEST_HXX

#include "module.hxx"         // defines module

namespace ngincc {
    namespace core {
        // forward declaration
        class plugin_manager;
        namespace shake {
            class test : public module {
            public:
                test(plugin_manager&pm);
                ~test();
            };
        }
    }
}

#endif // NGINZ_SHAKE_TEST_HXX
