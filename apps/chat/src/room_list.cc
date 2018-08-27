
#include <memory>
#include <functional>
#include <string>


#include "ngincc_config.h"
#include "log.hxx"
#include "plugin_manager.hxx"
#include "binary_coder.hxx"
#include "parallel/pipeline.hxx"
//#include "async_db.hxx"
//#include "async_db_master.hxx"
//#include "chat.h"
//#include "chat/chat_plugin_manager.h"
#include "chat/chat_connection.hxx"
#include "chat/chat_factory.hxx"
#include "chat/room_list.hxx"
//#include "chat/broadcast.h"
//#include "chat/room_list_master.hxx"

using std::string;
using std::vector;
using std::shared_ptr;

using ngincc::core::plugin_manager;
using ngincc::core::parallel::pipeline;
using ngincc::core::buffer_coder;
using ngincc::core::binary_coder;
using ngincc::apps::chat::chat_connection;
using ngincc::apps::chat::chat_factory;
using ngincc::apps::chat::room_list;
//using ngincc::apps::chat::room_list_master;
using namespace std::placeholders;


#if 0
static const char*ROOM_USER_KEY = "room/user/";
int chat_room_convert_room_from_room_pid_key(aroop_txt_t*room_key, aroop_txt_t*room) {
	aroop_assert(room_key);
	aroop_assert(room);
	aroop_txt_embeded_buffer(room, 64); // XXX it can break any time !
	aroop_txt_concat(room, room_key);
	int len = strlen(ROOM_PREFIX);
	aroop_txt_shift(room, len);
	return 0;
}

int chat_room_set_user_count(aroop_txt_t*my_room, int user_count, int increment) {
	if(aroop_txt_is_empty(my_room)) {
		return -1;
	}
	// get the number of users in that room
	aroop_txt_t db_room_key = {};
	aroop_txt_embeded_stackbuffer(&db_room_key, 128);
	aroop_txt_concat_string(&db_room_key, ROOM_USER_KEY);
	aroop_txt_concat(&db_room_key, my_room);
	aroop_txt_zero_terminate(&db_room_key);

	async_db_increment(-1, NULL, &db_room_key, user_count, increment);
	return 0;
}

int room_list::chat_room_get_pid(const string& room, uint32_t token, const string& response_hook) {
	if(room.size() == 0) {
		return -1;
	}

    stringstream builder;
    builder << ROOM_PREFIX << room;
    adb_client.get(token, response_hook, builder.c_str());
	return 0;
}
#endif

int room_list::lookup_rooms(vector<string>& cmd_args, chat_connection& chat) {
    return core_pipe.async_request_master(
        chat.get_token()
        , room_list::on_async_reply
        , room_list::on_async_request, {});
}

int room_list::on_room_list_retrieval(buffer_coder& recv_buffer) {
	// 0 = srcpid, 1 = command, 2 = token, 3 = success, 4 = info
    uint32_t srcpid;
    string service;
    uint32_t reply_token;
    uint32_t reply_status;
	string info;
    binary_coder coder(recv_buffer);

    // canary check
    string can_start;
    coder >>= can_start;
    if(can_start != binary_coder::canary_begin) {
        throw std::underflow_error("on_room_list_retrieval:canary check failed");
    }

    coder >>= srcpid;
    coder >>= service;
    coder >>= reply_token;
    coder >>= reply_status;
    coder >>= info;
    
    if(factory.has_chat(reply_token)) {
        auto& chat = factory.get_chat(reply_token); // needs cleanup
        if(info.size() == 0) {
            info = "There is no room\n";
        }
        chat->net_send(info, 0);
    } else {
        syslog(LOG_ERR, "Error, could not find the chat user:%u\n", reply_token);
    }
	return 0;
}

room_list::room_list(
    plugin_manager& core_plug
    , plugin_manager& chat_plug
    , pipeline& core_pipe
    , chat_factory& factory
    ) : core_plug(core_plug)
    , chat_plug(chat_plug)
    , core_pipe(core_pipe)
    , factory(factory) {
	//if(is_master())
	//	room_master_module_init();

    // listen for reply
    std::function<int(buffer_coder&)> async_reply_callback = std::bind(&room_list::on_room_list_retrieval, this, _1);
    core_plug.plug_add(string(on_async_reply), "It responds to asynchronous response from db.", std::move(async_reply_callback));

    // register chat command
    std::function<int(vector<string>&, chat_connection&)> lookup_rooms_cb = std::bind(&room_list::lookup_rooms, this, _1, _2);
    chat_plug.plug_add("state/logged_in/rooms", "Dump list of chat-rooms", std::move(lookup_rooms_cb));
    chat_plug.plug_add("state/in_room/rooms", "Dump list of chat-rooms", std::move(lookup_rooms_cb));
}

room_list::~room_list() {
    /* TODO
	if(is_master())
		room_master_module_deinit();
	composite_unplug_bridge(chat_plugin_manager_get(), 0, chat_room_lookup_plug);
	pm_unplug_callback(0, on_room_list_retrieval);
    */
}


