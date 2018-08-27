
#include <memory>
#include <string>
#include <vector>

#include <unistd.h>

#include "ngincc_config.h"
#include "log.hxx"
#include "plugin_manager.hxx"
#include "binary_coder.hxx"
#include "parallel/pipeline.hxx"
#include "chat/chat_connection.hxx"
#include "chat/chat_connection_state.hxx"
#include "chat/broadcast_room.hxx"

using std::string;
using std::vector;
using std::endl;
using std::ostringstream;
using std::shared_ptr;

using ngincc::apps::chat::chat_connection;
using ngincc::apps::chat::connection_state;
using ngincc::apps::chat::connection_state_logged_in;
using ngincc::apps::chat::broadcast_room;
using ngincc::apps::chat::broadcast_room_module;
using namespace std::placeholders;

const string& broadcast_room::get_name() const {
    return name;
}

const unsigned int broadcast_room::connection_count() const {
    return user_count;
}

//! \brief This implementation of private-message is O(n)
int broadcast_room::send_private(
    shared_ptr<chat_connection>& src
    , const string& to, const string& msg) {
    if(name == "") {
        // default room, none is here
        return 0;
    }
    for(auto xuser : user_list) {
        if(xuser->get_login_name() == to) { // TODO optimize this comparison
            xuser->net_send(src->get_login_name(), ngincc::net::net_channel::NET_MSG_MORE);
            xuser->net_send(": ", ngincc::net::net_channel::NET_MSG_MORE);
            xuser->net_send(msg, ngincc::net::net_channel::NET_MSG_MORE);
            xuser->net_send("\n", 0);
            break;
        }
    }
	return 0;
}

int broadcast_room::send_all(chat_connection& chat, const string& msg) {
    for(auto xuser : user_list) {
        if(*xuser == chat) {
            return send_all(xuser, msg);
        }
    }
	return 0;
}

int broadcast_room::send_all(
    shared_ptr<chat_connection>& src
    , const string& msg) {
    if(name == "") {
        // default room, none is here
        return 0;
    }

    //ostringstream prefix;
    //prefix << src->get_name() << ": ";

    for(auto xuser : user_list) {
        if(*xuser == *src) { // TODO optimize this comparison
            continue;
        }
        xuser->net_send(src->get_login_name(), ngincc::net::net_channel::NET_MSG_MORE);
        xuser->net_send(": ", ngincc::net::net_channel::NET_MSG_MORE);
        xuser->net_send(msg, ngincc::net::net_channel::NET_MSG_MORE);
        xuser->net_send("\n", 0);
    }
	return 0;
}


int broadcast_room::add_connection_and_greet(shared_ptr<chat_connection>& chat) {

    // check if the user is already in the user-list
    if(find(user_list.begin(), user_list.end(), chat) != user_list.end()) {
		syslog(LOG_ERR, "User is already in the room-list\n");
        return -1;
    }
    user_list.emplace_front(chat);
    user_count++;

    // say greetings
    ostringstream output;
    output << "Entering room:" << get_name() << endl;
	syslog(LOG_ERR, "Sending greetings from the chat-room\n");
	
    chat->net_send(output.str(), ngincc::net::net_channel::NET_MSG_MORE);

	// now print the user list ..
    for(auto xuser : user_list) {

        output.str("");
        output.clear();

        output << "\t* " << xuser->get_login_name();
        if(*xuser == *chat) {
            output << "(** this is you)";
        }
        output << endl;
		chat->net_send(output.str(), ngincc::net::net_channel::NET_MSG_MORE);
    }

	// end
	chat->net_send("end of list\n", 0);

	// broadcast that there is new user
    output.str("");
    output.clear();
    output << "\t* user joined chat" << endl;
    send_all(chat, output.str());
	return 0;
}

int broadcast_room::leave(shared_ptr<chat_connection>& chat) {
    for(auto xuser : user_list) {
        if(*xuser == *chat) {
            user_count--; // do accounting
            break;
        }
    }
    user_list.remove(chat);
    ostringstream output;
    output << "\t* user has left chat:";
    send_all(chat, output.str());
	return 0;
}

int broadcast_room::leave(chat_connection& chat) {
    shared_ptr<chat_connection> chat_ptr;
    for(auto xuser : user_list) {
        if(*xuser == chat) {
            user_count--; // do accounting
            chat_ptr = xuser;
            break;
        }
    }
    user_list.remove(chat_ptr);
    ostringstream output;
    output << "\t* user has left chat:";
    send_all(chat_ptr, output.str());
	return 0;
}

broadcast_room::broadcast_room() : name(""), user_count(0) {
}

broadcast_room::broadcast_room(string name) : name(name), user_count(0) {
}

bool broadcast_room::operator ==(const broadcast_room& other) const {
    return (this == &other) || (name == other.name);
}

int broadcast_room_module::send_all(
    shared_ptr<chat_connection>& src
    , const string& msg
    ) {
    broadcast_room& room = get_room(src->get_token());
    return room.send_all(src, msg);
}

int broadcast_room_module::send_all(
    chat_connection& src
    , const string& msg
    ) {
    broadcast_room& room = get_room(src.get_token());
    return room.send_all(src, msg);
}



int broadcast_room_module::join(
    shared_ptr<chat_connection>& chat, const string& room_name) {
	/* sanity check */
	if(!chat->in_state(typeid(connection_state_logged_in))) {
		syslog(LOG_ERR, "The user tries to join a room while he is not logged in\n");
		return -1;
	}
    // check if the user is already in the room
    if(get_room(chat->get_token()).get_name() == room_name) {
	    chat->net_send("Already joined\n", 0);
        return 0;
    }
	// find the chatroom
    const auto result = room_list.find(room_name);
	if(result == room_list.end()) {
		syslog(LOG_ERR, "Cannot find room %s\n", room_name.c_str());
		return -1;
	}
    broadcast_room& room = result->second;
	if(room.connection_count() >= NGINZ_MAX_CHAT_ROOM_USER) {
	    chat->net_send("Sorry, room is full\n", 0);
		return -1;
	}
    // leave the old room
    leave(chat);
    while(room_of_user.size() < chat->get_token()) {
        room_of_user.push_back(default_room);
    }
    room_of_user[chat->get_token()] = room;
    room.add_connection_and_greet(chat);
	return 0;
}

int broadcast_room_module::leave(chat_connection& chat) {
	// find the chatroom
    broadcast_room room = get_room(chat.get_token());
	if(room == default_room) {
		// syslog(LOG_ERR, "We do not know how he can leave room");
		return 0;
	}
	room.leave(chat);

	// prune the user from the list
    room_of_user[chat.get_token()] = default_room;
	return 0;
}

int broadcast_room_module::leave(shared_ptr<chat_connection>& chat) {
	// find the chatroom
    broadcast_room room = get_room(chat->get_token());
	if(room == default_room) {
		syslog(LOG_ERR, "We do not know how he can leave room");
		return 0;
	}
	room.leave(chat);

	// prune the user from the list
    room_of_user[chat->get_token()] = default_room;
	return 0;
}

int broadcast_room_module::add_room(const string& room_name) {
	// CHECK if the room already exists
    const auto result = room_list.find(room_name);
	if(result != room_list.end()) {
		syslog(LOG_ERR, "room already added");
		return 0;
	}

    // otherwise add new room
    room_list.emplace(room_name, room_name);
	syslog(LOG_NOTICE, "[%d]Adding room %s\n", getpid(), room_name.c_str());
	return 0;
}

broadcast_room& broadcast_room_module::get_room(uint32_t chat_connection_token) {
    while(room_of_user.size() <= chat_connection_token) {
        room_of_user.push_back(default_room);
    }
    return room_of_user[chat_connection_token];
}



