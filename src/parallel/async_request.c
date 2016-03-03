
#include <aroop/aroop_core.h>
#include <aroop/core/thread.h>
#include <aroop/core/xtring.h>
#include <aroop/opp/opp_factory.h>
#include <aroop/opp/opp_factory_profiler.h>
#include <aroop/opp/opp_hash_table.h>
#include "nginz_config.h"
#include "log.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "event_loop.h"
#include "parallel/pipeline.h"
#include "parallel/async_request.h"

C_CAPSULE_START

#define DB_LOG(...) syslog(__VA_ARGS__)

static aroop_txt_t null_hook = {};
static int masterpid = 0;
int async_pm_call_master(int cb_token, aroop_txt_t*worker_hook, aroop_txt_t*master_hook, aroop_txt_t*args[]) { // NULL terminated array
	if(worker_hook == NULL) {
		worker_hook = &null_hook;
	}
	aroop_txt_t bin = {}; // binary response
	// send response
	aroop_txt_embeded_stackbuffer(&bin, 255); // XXX it may overflow any time
	binary_coder_reset(&bin);
	// 0 = pid, 1 = srcpid, 2 = command, 3 = token, 4 = worker_hook, 5 = key, 6 = newval, 7 = oldval
	binary_pack_int(&bin, masterpid); // send destination pid
	binary_pack_int(&bin, getpid()); // send source pid
	binary_pack_string(&bin, master_hook);
	binary_pack_int(&bin, cb_token); // id/token
	binary_pack_string(&bin, worker_hook); // callback hook
	if(args)while(*args) {
		binary_pack_string(&bin, *args); // pack more ..
		args++;
	}
	DB_LOG(LOG_NOTICE, "[token%d]-request-[master:%d]-[app:%s]", cb_token, masterpid, aroop_txt_to_string(master_hook));
	pp_bubble_up(&bin);
	return 0;
}

int async_pm_reply_worker(int destpid, int cb_token, aroop_txt_t*worker_hook, aroop_txt_t*master_hook, int success, aroop_txt_t*args[]) { // NULL terminated array
	aroop_assert(!aroop_txt_is_empty_magical(worker_hook));
	aroop_assert(!aroop_txt_is_empty_magical(master_hook));
	aroop_txt_t bin = {}; // binary response
	// send response
	// 0 = pid, 1 = src pid, 2 = command, 3 = token, 4 = worker_hook, 5 = success
	aroop_txt_embeded_stackbuffer(&bin, 255);
	binary_coder_reset(&bin);
	binary_pack_int(&bin, destpid); // send destination pid
	binary_pack_int(&bin, getpid()); // send destination pid
	binary_pack_string(&bin, master_hook);
	binary_pack_int(&bin, cb_token); // id/token
	binary_pack_string(&bin, worker_hook); // id/token
	binary_pack_int(&bin, success); // means success
	if(args)while(*args) {
		binary_pack_string(&bin, *args); // pack more ..
		args++;
	}
	DB_LOG(LOG_NOTICE, "[token%d]-response--[dest:%d][app:%s]", cb_token, destpid, aroop_txt_to_string(master_hook));
	pp_bubble_down(&bin);
	return 0;
}

int async_request_init() {
	aroop_txt_embeded_set_static_string(&null_hook, "null");
	masterpid = getpid();
	return 0;
}

int async_request_deinit() {
	return 0;
}



C_CAPSULE_END
