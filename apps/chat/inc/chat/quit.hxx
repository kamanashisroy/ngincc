#ifndef NGINCC_CHAT_QUIT_HXX
#define NGINCC_CHAT_QUIT_HXX

namespace ngincc {
    namespace core {
        // forward declaration
        class plugin_manager;
    }
    namespace apps {
        namespace chat {

            // forward declaration
            class chat_connection;

            class quit {
            public:
                quit(ngincc::core::plugin_manager& chat_plug);
                ~quit();

            private:
                ngincc::core::plugin_manager& chat_plug;
                int process_quit(std::vector<std::string>& cmd_args, chat_connection& chat);
            };
        }
    }
}

#endif // NGINCC_CHAT_QUIT_HXX
