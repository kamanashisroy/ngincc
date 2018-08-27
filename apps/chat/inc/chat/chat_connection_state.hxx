#ifndef NGINCC_APPS_CHAT_CONNECTION_STATE_HXX
#define NGINCC_APPS_CHAT_CONNECTION_STATE_HXX

#include <string>
#include <sstream>

#include "binary_coder.hxx"

namespace ngincc {
    namespace core {
        // forward declaration
        class event_loop;
        class plugin_manager;
        class buffer_coder;
    }
    namespace db {
        class async_db;
    }
    namespace apps {
        namespace chat {
            // forward declaration
            class chat_connection;
            class chat_factory;
            class broadcast_room_module;
            //! Connection state supports different behavior for different
            //! chat state
            class connection_state {
            public:
                virtual ~connection_state() = default;
                //! \brief process client request line
                virtual int process_chat_request(chat_connection& chat, const std::string& request) = 0;
                virtual const std::string to_string() const = 0;
            };
            class connection_state_connected final : public connection_state {
            public:
                connection_state_connected() = delete;
                connection_state_connected(
                    ngincc::core::plugin_manager& core_plug
                    , ngincc::core::plugin_manager &chat_plug
                    , chat_factory& factory
                    , ngincc::db::async_db& adb_client
                );
                virtual ~connection_state_connected() override;
                virtual int process_chat_request(chat_connection& chat, const std::string& request) override;
                virtual const std::string to_string() const override;
            private:
                ngincc::core::plugin_manager &chat_plug;
                chat_factory& factory;
                ngincc::db::async_db& adb_client;

                int try_login(chat_connection& chat, const std::string& name);
                int async_try_login(const std::string& name, uint32_t token, std::string&& response_hook);
                int async_logoff(const std::string& name);
                int on_login_reply(ngincc::core::buffer_coder& recv_buffer);
            };
            class connection_state_logged_in final : public connection_state {
            public:
                connection_state_logged_in() = delete;
                connection_state_logged_in(
                    ngincc::core::plugin_manager &chat_plug
                    , chat_factory& factory
                    , ngincc::db::async_db& adb_client
                );
                virtual ~connection_state_logged_in() override;
                virtual int process_chat_request(chat_connection& chat, const std::string& request) override;
                virtual const std::string to_string() const override;
            private:
                ngincc::core::plugin_manager &chat_plug;
                chat_factory& factory;
                ngincc::db::async_db& adb_client;
            };
            class connection_state_in_room final : public connection_state {
            public:
                connection_state_in_room() = delete;
                connection_state_in_room(
                    ngincc::core::plugin_manager &chat_plug
                    , chat_factory& factory
                    , ngincc::db::async_db& adb_client
                    , broadcast_room_module& bcast_module
                );
                virtual ~connection_state_in_room() override;
                virtual int process_chat_request(chat_connection& chat, const std::string& request) override;
                virtual const std::string to_string() const override;
            private:
                ngincc::core::plugin_manager &chat_plug;
                chat_factory& factory;
                ngincc::db::async_db& adb_client;
                broadcast_room_module& bcast_module;
            };
            class connection_state_quitting final : public connection_state {
            public:
                connection_state_quitting() = delete;
                connection_state_quitting(
                    ngincc::core::plugin_manager &chat_plug
                );
                virtual ~connection_state_quitting() override;
                virtual int process_chat_request(chat_connection& chat, const std::string& request) override;
                virtual const std::string to_string() const override;
            private:
                ngincc::core::plugin_manager &chat_plug;
            };
        }
    }
}

#endif // NGINCC_APPS_CHAT_CONNECTION_STATE_HXX
