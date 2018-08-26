
#include <string>
#include <vector>
#include <unistd.h> // declares getpid()


#include "ngincc_config.h"
#include "log.hxx"
#include "plugin_manager.hxx"
#include "binary_coder.hxx"
#include "parallel/pipeline.hxx"
#include "async_db.hxx"
#include "async_db_master.hxx"
//#include "chat.h"
//#include "chat/chat_plugin_manager.h"
#include "chat/room_list.hxx"
//#include "chat/broadcast.h"
#include "chat/room_list_master.hxx"

using std::string;
using std::vector;
using std::ostringstream;
using std::istringstream;
using std::stringstream;
using std::endl;

using ngincc::core::plugin_manager;
using ngincc::core::buffer_coder;
using ngincc::core::binary_coder;
using ngincc::core::parallel::pipeline;
using ngincc::db::async_db_master;
using ngincc::apps::chat::room_list;
using ngincc::apps::chat::room_list_master;
using namespace std::placeholders;

static inline const std::array<std::string, 10> default_rooms = {"ONE", "TWO", "THREE", "FOUR", "FIVE", "SIX", "SEVEN", "EIGHT", "NINE", "TEN"};
static inline const string ROOM_USER_KEY {"room/user/"};
static inline const string ACTIVE_ROOMS_KEY {"rooms"};

int room_list_master::get_user_count(const string& xroom) {
    string countstr;
    stringstream builder;
    builder << ROOM_USER_KEY << xroom;
	adb_master.noasync_get(builder.str(), countstr); // needs cleanup
    if(countstr.size() == 0) {
        return 0;
    }
	return std::stoi(countstr);
}

int room_list_master::list_rooms_hook(buffer_coder& recv_buffer) {
	// 0 = srcpid, 1 = command, 2 = token, 3 = reply_path, 4 = key, 5 = newval, 6 = oldval
    uint32_t srcpid;
    string service;
    uint32_t reply_token;
    string reply_path;
    ostringstream builder;
    binary_coder coder(recv_buffer);

    // canary check
    string can_start;
    coder >>= can_start;
    if(can_start != binary_coder::canary_begin) {
        throw std::underflow_error("list_rooms_hook:canary check failed");
    }

    coder >>= srcpid;
    coder >>= service;
    coder >>= reply_token;
    coder >>= reply_path;
    
	builder << "Active rooms are:" << endl;
    string active_rooms;
    adb_master.noasync_get(ACTIVE_ROOMS_KEY, active_rooms);

    istringstream room_reader(active_rooms);
    char delim = ' ';
    for(string xroom; room_reader >> xroom; room_reader >> delim) {
        if(xroom.size() == 0) {
            continue;
        }
	    int count = get_user_count(xroom);
        builder << xroom << ' ' << count << endl;
	}
    builder << "end of list." << endl;
    
    string response = builder.str();
	//syslog(LOG_ERR, "rooms:%s\n", aroop_txt_to_string(&room_info));
    core_pipe.async_reply_worker(srcpid, reply_token, reply_path, 1, {response});
	return 0;
}

int room_list_master::master_init(vector<string>& , ostringstream& output) {
    ostringstream builder;
    for(string room : default_rooms) {
        builder << room << ' ';
    }
    adb_master.noasync_set(ACTIVE_ROOMS_KEY, builder.str());
    output << "Initializing chat-rooms" << endl;

    // listen for request
    std::function<int(buffer_coder&)> request_cb = std::bind(&room_list_master::list_rooms_hook, this, _1);
    core_plug.plug_add(string(room_list::on_async_request), "Responses with room-list.", std::move(request_cb));
	return 0;
}

int room_list_master::make_room(unsigned int index) {
    if(index >= default_rooms.size()) {
        throw std::out_of_range("room index out of bound");
    }
    ostringstream builder;
    builder << room_list::ROOM_PREFIX << default_rooms[index];

    ostringstream object_builder;
    object_builder << getpid() << ' ' << default_rooms[index];
    adb_master.noasync_set(builder.str(), object_builder.str());

	// create the room constructs
	// TODO broadcast_add_room(&room_name);
	return 0;
}

int room_list_master::parent_after_hook(int child_pid) {
	internal_child_count+=2; // add default indexes
	return 0;
}

int room_list_master::child_after_hook(int pid) {
	make_room(internal_child_count++); // add default indexes
	make_room(internal_child_count); // add default indexes
	//make_room(internal_child_count++); // add default indexes
	return 0;
}

room_list_master::room_list_master(
    plugin_manager& core_plug
    , async_db_master& adb_master
    , pipeline& core_pipe
    ) : internal_child_count(0)
    , core_plug(core_plug)
    , adb_master(adb_master)
    , core_pipe(core_pipe) {

    std::function<int(int)> fork_cb = std::bind(&room_list_master::parent_after_hook, this, _1);
	core_plug.plug_add("fork/parent/after", "It assigns a room to a worker process.", std::move(fork_cb));

    fork_cb = std::bind(&room_list_master::child_after_hook, this, _1);
	core_plug.plug_add("fork/child/after", "It assigns a room to a worker process.", std::move(fork_cb));


    std::function<int(vector<string>&,ostringstream&)> master_init_cb = std::bind(&room_list_master::master_init, this, _1, _2);
	core_plug.plug_add("master/init", "It prepares the master hooks.", std::move(master_init_cb));
}

room_list_master::~room_list_master() {
	/* TODO aroop_assert(is_master());
	pm_unplug_bridge(0, default_room_fork_child_after_callback);
	pm_unplug_bridge(0, default_room_fork_parent_after_callback);
	pm_unplug_callback(0, on_async_room_call_master);
    */
}


