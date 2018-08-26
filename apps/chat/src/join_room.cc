
#include <memory>
#include <functional>
#include <string>
#include <unistd.h> // defines getpid()

#include "ngincc_config.h"
#include "log.hxx"
#include "module.hxx"
#include "binary_coder.hxx"
#include "plugin_manager.hxx"
#include "async_db.hxx"
#include "chat/chat_connection.hxx"
#include "chat/chat_factory.hxx"
#include "chat/room_list.hxx"
#include "chat/join_room.hxx"

using std::string;
using std::vector;
using std::endl;
using std::unique_ptr;
using std::istringstream;
using std::stringstream;

using ngincc::core::plugin_manager;
using ngincc::core::buffer_coder;
using ngincc::core::binary_coder;
using ngincc::db::async_db;
using ngincc::apps::chat::chat_connection;
using ngincc::apps::chat::connection_state;
using ngincc::apps::chat::connection_state_logged_in;
using ngincc::apps::chat::join_room;
using namespace std::placeholders;

static inline const string on_asyncchat_room_pid {"on/asyncchat/room/pid"};

#if 0
static int chat_join_transfer(struct chat_connection*chat, aroop_txt_t*room, int pid) {
	// leave current chatroom
	broadcast_room_leave(chat);
	// remove it from listener
	event_loop_unregister_fd(chat->strm.fd);
	aroop_txt_t cmd = {};
	aroop_txt_embeded_stackbuffer(&cmd, 128);
	aroop_txt_concat_string(&cmd, "chat/_hiddenjoin ");
	aroop_txt_concat(&cmd, room);
	aroop_txt_concat_string(&cmd, " ");
	aroop_txt_concat(&cmd, &chat->name);
	aroop_txt_concat_string(&cmd, "\n");
	aroop_txt_zero_terminate(&cmd);
	chat->strm.transfer_parallel(&chat->strm, pid, NGINZ_CHAT_PORT, &cmd);
	chat->state |= CHAT_SOFT_QUIT; // quit the user from this process
	//chat->strm.close(&chat->strm);
	lazy_cleanup(chat);
	return 0;
}
#endif

int join_room::join_helper(unique_ptr<chat_connection>& chat, string& room, int pid) {
	// check if it is same process
	//printf("target pid=%d, my pid = %d\n", pid, getpid());
	if(pid != getpid()) {
        chat->net_send( "This transfer feature is not available now! Sorry, please try another room.", 0);
#if 0
		chat_join_transfer(chat, room, pid);
#endif
		return 0;
	}
	//printf("assiging to room %s\n", aroop_txt_to_string(room));
	// otherwise assign to the room
    // TODO broadcast.room_join(chat,room);
    chat->set_state(factory.get_in_room());
	return 0;
}

#if 0
static int chat_join_get_room(aroop_txt_t*request, aroop_txt_t*room) {
	aroop_txt_t request_sandbox = {};
	aroop_txt_t token = {};
	int reqlen = aroop_txt_length(request);
	aroop_txt_embeded_stackbuffer(&request_sandbox, reqlen);
	aroop_txt_concat(&request_sandbox, request);
	scanner_next_token(&request_sandbox, &token);
	if(aroop_txt_is_empty(&token)) {
		return -1;
	}
	aroop_txt_embeded_rebuild_copy_on_demand(room, &token); // needs cleanup
	aroop_txt_zero_terminate(room);
	return 0;
}
#endif

int join_room::on_target_pid_retrieval(buffer_coder& recv_buffer) {
    uint32_t srcpid;
    string service;
    uint32_t reply_token;
    uint32_t reply_status;
    string room_key;
    string room_object;
	// 0 = srcpid, 1 = command, 2 = token, 3 = success, 4 = key, 5 = newvalue
    binary_coder coder(recv_buffer);


    // canary check
    string can_start;
    coder >>= can_start;
    if(can_start != binary_coder::canary_begin) {
        throw std::underflow_error("on_target_pid_retrieval:canary check failed");
    }

    coder >>= srcpid;
    coder >>= service;
    coder >>= reply_token;
    coder >>= reply_status;
    coder >>= room_key;
    coder >>= room_object;

    int pid = 0;
    [[maybe_unused]] char delim = ' ';
    string room_name;
    istringstream room_reader(room_object);
    room_reader >> pid >> delim >> room_name;
    
	//syslog(LOG_NOTICE, "------------ ..............  \n");
	//syslog(LOG_NOTICE, "Joining ..............  %d to %s\n", reply_token, room_key.c_str());
	//chat_room_convert_room_from_room_pid_key(room_key, room_val);
	//syslog(LOG_NOTICE, "Joining ..............  %d to %s\n", cb_token, room.c_str());
    if(factory.has_chat(reply_token)) {
        unique_ptr<chat_connection>& chat = factory.get_chat(reply_token);
        if(chat->in_state(typeid(connection_state_logged_in))) {
            if(reply_status && room_name.size() != 0) {
		        chat->net_send("Trying ...", ngincc::net::net_channel::NET_MSG_MORE);
		        join_helper(chat, room_name, pid);
            } else {
			    chat->net_send("The room is not avilable\n", 0);
            }
        } else {
			syslog(LOG_ERR, "Join failed, chat object not found %d\n", reply_token);
        }
    }
	return 0;
}

int join_room::process_join(vector<string>& cmd_args, chat_connection& chat) {
    if(cmd_args.size() <= 1 || cmd_args[1].size() == 0) {
	    chat.net_send("The room is not avilable\n", 0);
        return 0;
    }
	chat.net_send("Trying ...\n", MSG_MORE);
    stringstream builder;
    builder << room_list::ROOM_PREFIX << cmd_args[1];
    adb_client.get( chat.get_token(), on_asyncchat_room_pid, builder.str());
	return 0;
}

join_room::join_room(
    plugin_manager& core_plug
    , plugin_manager& chat_plug
    , async_db& adb_client
    , chat_factory& factory
    ) : core_plug(core_plug)
    , chat_plug(chat_plug)
    , adb_client(adb_client)
    , factory(factory)
    {

    // make join command
    std::function<int(vector<string>&,chat_connection&)> join_cb = std::bind(&join_room::process_join, this, _1, _2);
    //chat_plug.plug_add("state/in_room/join", "Join a room", std::move(join_cb));
    chat_plug.plug_add("state/logged_in/join", "Join a room", std::move(join_cb));


    // make asynchronous hook to process the pid-retrieval of the room

    std::function<int(buffer_coder&)> async_cb = std::bind(&join_room::on_target_pid_retrieval, this, _1);
    core_plug.plug_add(string(on_asyncchat_room_pid), "It asynchronously responds to join request.", std::move(async_cb));
}

join_room::~join_room() {
    //! \brief TODO unregister all
}


