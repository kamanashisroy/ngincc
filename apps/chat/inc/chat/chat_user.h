#ifndef NGINCC_CHAT_USER_HXX
#define NGINCC_CHAT_USER_HXX

namespace ngincc {
    namespace core {
        // forward declaration
        class plugin_manager;
    }
    namespace apps {
        namespace chat {
            class chat_connection;
            //! \brief chat_user allow's user to login and log-off
            class chat_user {
            public:
                chat_user() = delete;
                chat_user(ngincc::core::plugin_manager& chat_plug);
                int logoff(); 
            private:
                ngincc::core::plugin_manager& chat_plug;

                // hooks
                int on_client_welcome(std::vector<std::string>& args, chat_connection& chat);  //!< greet
                int on_client_login(std::vector<std::string>& args, chat_connection& chat); //!< handle login-reply

                // async api
                int async_try_login(const string& name, const string& reply_path);
            };
        }
    }
}
int try_login(aroop_txt_t*name);
int logoff_user(struct chat_connection*chat);

#define USER_PREFIX "user/"

#endif // NGINCC_CHAT_USER_HXX
