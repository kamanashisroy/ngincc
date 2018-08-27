#ifndef NGINCC_BROADCAST_HXX
#define NGINCC_BROADCAST_HXX

#include <memory>
#include <vector>
#include <unordered_map>
#include <forward_list>
#include <string>

namespace ngincc {
    namespace apps {
        namespace chat {
            class chat_connection;
            class broadcast_room_module;

            //! \brief A broadcast_room reprsents a chat-room and allows to broadcast
            //! public messages to room-users
            class broadcast_room {
            public:
                broadcast_room(std::string name);
                broadcast_room();
                ~broadcast_room() = default;
            private:
                bool operator ==(const broadcast_room& other) const;
                std::string name;
                std::forward_list<std::shared_ptr<chat_connection>> user_list;
                unsigned int user_count;
                friend broadcast_room_module;

                int send_all(chat_connection& src, const std::string& msg);
                int send_all(std::shared_ptr<chat_connection>& src, const std::string& msg);
                int send_private(std::shared_ptr<chat_connection>& src
                    , const std::string& to_user
                    , const std::string& msg);
                int add_connection_and_greet(std::shared_ptr<chat_connection>& chat);
                int leave(chat_connection& chat);
                int leave(std::shared_ptr<chat_connection>& chat);
                //! \brief getters
                inline const std::string& get_name() const;
                inline const unsigned int connection_count() const;
            };

            //! \brief broadcast_room_module allows create rooms and lookup rooms by name
            class broadcast_room_module {
            public:
                broadcast_room_module() = default;
                ~broadcast_room_module() = default;
                int add_room(const std::string& room_name);
                int send_all(chat_connection& src, const std::string& msg);
                int send_all(std::shared_ptr<chat_connection>& src, const std::string& msg);
                int send_private(std::shared_ptr<chat_connection>& src
                    , const std::string& to_user
                    , const std::string& msg);
                int join(std::shared_ptr<chat_connection>& chat, const std::string& room_name);
                int leave(chat_connection& chat);
                int leave(std::shared_ptr<chat_connection>& chat);
            private:
                broadcast_room default_room;
                std::unordered_map<std::string,broadcast_room> room_list;
                //! \brief the index of the vector is the user-connection token
                //! and the content of the vector is room reference
                std::vector<std::reference_wrapper<broadcast_room>> room_of_user;
                broadcast_room& get_room(uint32_t chat_connection_token);
            };
        }
    }
}

#endif // NGINCC_BROADCAST_HXX
