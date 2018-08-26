#ifndef NGINCC_CHAT_HELP_HXX
#define NGINCC_CHAT_HELP_HXX

namespace ngincc {
    namespace core {
        // forward declaration
        class plugin_manager;
    }
    namespace apps {
        namespace chat {

            // forward declaration
            class chat_connection;

            class help {
            public:
                help(ngincc::core::plugin_manager& chat_plug);
                ~help();

            private:
                ngincc::core::plugin_manager& chat_plug;
                int process_help(std::vector<std::string>& cmd_args, chat_connection& chat);
            };
        }
    }
}

#endif // NGINCC_CHAT_HELP_HXX
