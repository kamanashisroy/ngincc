
#include <functional> // defines std::ref
#include <string>
#include <initializer_list>

#include "log.hxx"
#include "async_db.hxx"
#include "async_db_master.hxx"
#include "parallel/pipeline.hxx"


#define DB_LOG(...) syslog(__VA_ARGS__)
//#define DB_LOG(...)

using std::string;
using ngincc::db::async_db;
using ngincc::db::async_db_master;


int async_db::compare_and_swap(uint32_t cb_token, const string& cb_hook, const string& key, const string& newval, const string& oldval) {
    if(0 == key.size()) { // sanity check
        throw std::invalid_argument("Db: key cannot be null");
    }
    /*if(0 != oldval.size() && 0 == newval.size()) {
        throw std::invalid_argument("Db: cannot set null to existing value(use unset instead)");
    }*/
	// 0 = pid, 1 = srcpid, 2 = command, 3 = token, 4 = cb_hook, 5 = key, 6 = newval, 7 = oldval
	DB_LOG(LOG_NOTICE, "[token%d]-CAS-throwing to--[master]-[key:%s]", cb_token, key.c_str());
	core_pipe.async_request_master(cb_token
        , cb_hook
        , (0 == oldval.size()) ? ((0 == newval.size()) ? async_db_master::unset_app : async_db_master::set_if_null_app) : async_db_master::compare_and_swap_app
        , {std::ref(key), std::ref(newval), std::ref(oldval)});
	return 0;
}

int async_db::unset(uint32_t cb_token, const string& cb_hook, const string& key) {
	return compare_and_swap(cb_token, cb_hook, key, async_db_master::null_value, async_db_master::null_value);
}

int async_db::set_if_null(uint32_t cb_token, const string& cb_hook, const string& key, uint32_t intval) {
	return compare_and_swap(cb_token, cb_hook, key, std::to_string(intval), async_db_master::null_value);
}


int async_db::increment(const uint32_t cb_token, const string& cb_hook, const string& key, const uint32_t intval, const uint32_t increment) {
	if((intval + increment) == 0) {
		return async_db::unset(cb_token, cb_hook, key);
	}

	if(intval == 0) {
		return async_db::set_if_null(cb_token, cb_hook, key, increment);
    }

    // TODO optimize and use binary-int/decimal types
    string oldstr = std::to_string(intval);
    string newstr = std::to_string(intval+increment);
	compare_and_swap(cb_token, cb_hook, key, newstr, oldstr);
	return 0;
}

int async_db::get(uint32_t cb_token, const string& cb_hook, const string& key) {
    if(0 == key.size()) { // sanity check
        throw std::invalid_argument("Db: key cannot be null");
    }
	// 0 = pid, 1 = srcpid, 2 = command, 3 = token, 4 = cb_hook, 5 = key
	DB_LOG(LOG_NOTICE, "[token%d]-get-throwing to--[master]-[key:%s]", cb_token, key.c_str());
	return core_pipe.async_request_master(cb_token, cb_hook, async_db_master::get_app, {key});
}

async_db::async_db(
    ngincc::core::parallel::pipeline& core_pipe)
    : core_pipe(core_pipe) {
}

