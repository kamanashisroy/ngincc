
#include <functional> // defines std::ref
#include <initializer_list>
#include <stdexcept>

#include "log.hxx"
#include "binary_coder.hxx"
#include "plugin_manager.hxx"
#include "parallel/pipeline.hxx"
#include "async_db.hxx"
#include "async_db_master.hxx"

using std::string;
using std::ostringstream;
using std::vector;
using std::domain_error;
using ngincc::core::buffer_coder;
using ngincc::core::binary_coder;
using ngincc::core::plugin_manager;
using ngincc::core::parallel::pipeline;
using ngincc::db::async_db;
using ngincc::db::async_db_master;
using namespace std::placeholders;


#define DB_LOG(...) syslog(__VA_ARGS__)
//#define DB_LOG(...)

/*int noasync_db_get(aroop_txt_t*key, aroop_txt_t*val) {
	aroop_assert(is_master());
	aroop_txt_t*oldval = (aroop_txt_t*)opp_hash_table_get_no_ref(&global_db, key); // no cleanup needed
	if(oldval)
		aroop_txt_embeded_rebuild_copy_shallow(val, oldval); // needs cleanup
	return 0;
}*/

int async_db_master::reply_helper(uint32_t destpid, uint32_t cb_token, const string& cb_hook, uint32_t success, const string& key, const string& newval) {
	// send response
	// 0 = pid, 1 = src pid, 2 = command, 3 = token, 4 = cb_hook, 5 = success
	DB_LOG(LOG_NOTICE, "[token%d]-replying-throwing to--[dest:%d]-[key:%s]-[app:%s]", cb_token, destpid, key.c_str(), cb_hook.c_str());
	if(cb_hook.size() == 0) {
        // XXX do we need to reply ?
        // TODO optimize
		return core_pipe.async_reply_worker(destpid, cb_token, async_db::empty_hook, success, {std::ref(key),std::ref(newval)});
	}
	return core_pipe.async_reply_worker(destpid, cb_token, cb_hook, success, {key,newval});
}

bool async_db_master::cas_helper(const string& key, const string& newval, const string& expval) {
    if(key.size() == 0) { // sanity check
        return false;
    }
    bool success = false;
    auto it = data.find(key);
	if(newval.size() == 0) { // unset or set it to null
        if(it != data.end()) { // when found
            if(expval == it->second) {
                data.erase(it); // unset
                success = true;
            }
        } else { // when already empty
            success = true;
        }
	} else {
        // set if it is the same as expected value
        if(it != data.end()) {
            // when not empty
            if(it->second == expval) {
                it->second = newval; // update
                success = true;
            }
        }
    }
	DB_LOG(LOG_NOTICE, "--op----[key:%s]", key.c_str());
    return success;
}

bool async_db_master::sin_helper(const string& key, const string& newval) {
    if(key.size() == 0) { // sanity check
        return false;
    }
    bool success = false;
    auto it = data.find(key);
    if(it == data.end()) { // when not found
        data.insert(std::make_pair(key,newval));
        success = true;
    }
	DB_LOG(LOG_NOTICE, "--op----[key:%s]", key.c_str());
    return success;
}

bool async_db_master::unset_helper(const string& key) {
    if(key.size() == 0) { // sanity check
        return false;
    }
    bool success = false;
    auto it = data.find(key);
    if(it != data.end()) { // when found
        data.erase(it);
        success = true;
    }
	DB_LOG(LOG_NOTICE, "--op----[key:%s]", key.c_str());
    return success;
}

bool async_db_master::get_helper(const string& key, string& outval) {
    if(key.size() == 0) { // sanity check
        return false;
    }
    bool success = false;
    auto it = data.find(key);
    if(it != data.end()) { // when found
        outval = it->second;
        success = true;
    }
	DB_LOG(LOG_NOTICE, "--op----[key:%s]", key.c_str());
    return success;
}



int async_db_master::cas_hook(buffer_coder& recv_buffer) {
    if(!is_master) {
	    // The database is available only in the master process
        throw domain_error("The client does not have data");
    }
	string key; // the key to set
    string service; // unused
	string expval; // the val to compare with the oldval
	string newval; // the val to set
	uint32_t srcpid = 0;
	uint32_t cb_token = 0;
	string cb_hook;
	// 0 = srcpid, 1 = command, 2 = token, 3 = cb_hook, 4 = key, 5 = newval, 6 = oldval
    binary_coder coder(recv_buffer);

    // canary check
    string can_start;
    coder >>= can_start;
    if(can_start != binary_coder::canary_begin) {
        throw std::underflow_error("cas_hook:canary check failed");
    }

    coder >>= srcpid;
    coder >>= service;
    coder >>= cb_token;
    coder >>= cb_hook;
    coder >>= key;
    coder >>= newval;
    coder >>= expval;
	DB_LOG(LOG_NOTICE, "[token%d]-CAS-doing ..--[dest:%d]-[key:%s]-[app:%s]", cb_token, srcpid, key.c_str(), cb_hook.c_str());
	const uint32_t success = cas_helper(key, newval, expval)?1:0;
#if 0
	if(destpid > 0) {
#endif
		reply_helper(srcpid, cb_token, cb_hook, success, key, newval);
#if 0
	}
#endif
	return 0;
}

int async_db_master::sin_hook(buffer_coder& recv_buffer) {
    if(!is_master) {
	    // The database is available only in the master process
        throw domain_error("The client does not have data");
    }
	string key; // the key to set
    string service; // unused
	string newval; // the val to set
	uint32_t srcpid = 0;
	uint32_t cb_token = 0;
	string cb_hook;
    recv_buffer.seekpos(0);
    binary_coder coder(recv_buffer);

    // canary check
    string can_start;
    coder >>= can_start;
    if(can_start != binary_coder::canary_begin) {
        throw std::underflow_error("sin_hook:canary check failed");
    }

	// 0 = srcpid, 1 = service/command, 2 = token, 3 = cb_hook, 4 = key, 5 = newval, 6 = oldval
    coder >>= srcpid;
    coder >>= service;
    coder >>= cb_token;
    coder >>= cb_hook;
    coder >>= key;
    coder >>= newval;
	DB_LOG(LOG_NOTICE, "[token%d]-set if null-doing ..--[dest:%d]-[key:%s]-[app:%s]", cb_token, srcpid, key.c_str(), cb_hook.c_str());
	const uint32_t success = sin_helper(key, newval)?1:0;
#if 0
	if(destpid > 0) {
#endif
		reply_helper(srcpid, cb_token, cb_hook, success, key, newval);
#if 0
	}
#endif
	return 0;
}

int async_db_master::unset_hook(buffer_coder& recv_buffer) {
    if(!is_master) {
	    // The database is available only in the master process
        throw domain_error("The client does not have data");
    }
	string key; // the key to set
    string service; // unused
	uint32_t srcpid;
	uint32_t cb_token;
	string cb_hook;
    binary_coder coder(recv_buffer);

    // canary check
    string can_start;
    coder >>= can_start;
    if(can_start != binary_coder::canary_begin) {
        throw std::underflow_error("unset_hook:canary check failed");
    }

	// 0 = srcpid, 1 = command, 2 = token, 3 = cb_hook, 4 = key, 5 = newval, 6 = oldval
    coder >>= srcpid;
    coder >>= service;
    coder >>= cb_token;
    coder >>= cb_hook;
    coder >>= key;
	DB_LOG(LOG_NOTICE, "[token%d]-unset-doing ..--[dest:%d]-[key:%s]-[app:%s]", cb_token, srcpid, key.c_str(), cb_hook.c_str());
	const uint32_t success = unset_helper(key)?1:0;
#if 0
	if(destpid > 0) {
#endif
		reply_helper(srcpid, cb_token, cb_hook, success, key, async_db_master::null_value);
#if 0
	}
#endif
	return 0;
}

int async_db_master::get_hook(buffer_coder& recv_buffer) {
    if(!is_master) {
	    // The database is available only in the master process
        throw domain_error("The client does not have data");
    }
	string key; // the key to set
    string service; // unused
	uint32_t srcpid;
	uint32_t cb_token;
	string cb_hook;
    binary_coder coder(recv_buffer);

    // canary check
    string can_start;
    coder >>= can_start;
    if(can_start != binary_coder::canary_begin) {
        throw std::underflow_error("get_hook:canary check failed");
    }

	// 0 = srcpid, 1 = command, 2 = token, 3 = cb_hook, 4 = key, 5 = val
    coder >>= srcpid;
    coder >>= service;
    coder >>= cb_token;
    coder >>= cb_hook;
    coder >>= key;
	DB_LOG(LOG_NOTICE, "[token%d]-get-doing ..--[dest:%d]-[key:%s]-[app:%s]", cb_token, srcpid, key.c_str(), cb_hook.c_str());
	//syslog(LOG_NOTICE, "[pid:%d]-getting:%s", getpid(), aroop_txt_to_string(&key));
    string outval;
    const uint32_t success = get_helper(key,outval)?1:0;
	reply_helper(srcpid, cb_token, cb_hook, success, key, outval);
	return 0;
}

int async_db_master::db_dump(vector<string>& cmdargs, ostringstream& output) {
    for(auto record: data) {
        output << '[' << record.first << ']' << '>' << '[' << record.second << ']' << std::endl;
    }
	return 0;
}

//! \brief attach all the db-hooks
int async_db_master::start(vector<string>& cmd_args, ostringstream& output) {
    is_master = true;

    // subscribe to asyncdb/sin/request
    std::function<int(buffer_coder&)> callback = std::bind(&async_db_master::cas_hook,this,_1);
	core_plug.plug_add<buffer_coder&>(string(async_db_master::compare_and_swap_app), "It implements cas in master process.", move(callback));

    // subscribe to asyncdb/sin/request
    callback = std::bind(&async_db_master::sin_hook,this,_1);
	core_plug.plug_add<buffer_coder&>(string(async_db_master::set_if_null_app), "It implements set-if-null-app in master process.", move(callback));

    // subscribe to asyncdb/unset/request
    callback = std::bind(&async_db_master::unset_hook,this,_1);
	core_plug.plug_add<buffer_coder&>(string(async_db_master::unset_app), "It implements unset-app in master process.", move(callback));

    // subscribe to asyncdb/unset/request
    callback = std::bind(&async_db_master::get_hook,this,_1);
	core_plug.plug_add<buffer_coder&>(string(async_db_master::get_app), "It implements get-app in master process.", move(callback));

    // allow shake lookup
    std::function<int(vector<string>&,ostringstream&)> callback2 = std::bind(&async_db_master::db_dump,this,_1,_2);
	core_plug.plug_add("shake/dbdump", "It dumps the db entries from database.", move(callback2));
    return 0;
}

async_db_master::async_db_master(
    ngincc::core::plugin_manager& core_plug
    , ngincc::core::parallel::pipeline& core_pipe)
    : is_master(false), core_plug(core_plug), core_pipe(core_pipe)
     {
    // start the db-server
    std::function<int(vector<string>&,ostringstream&)> callback = 
        std::bind(&async_db_master::start,this,_1,_2);
    core_plug.plug_add("master/init", "It starts the tcp listener.", move(callback));
}

async_db_master::~async_db_master() {
	// aroop_assert(is_master());
    // TODO unplug all plugins
	// pm_unplug_callback(0, async_db_CAS_hook);
	// pm_unplug_callback(0, async_db_set_if_null_hook);
	// pm_unplug_callback(0, async_db_unset_hook);
	// pm_unplug_callback(0, async_db_get_hook);
	// pm_unplug_callback(0, async_db_dump_hook);
}


