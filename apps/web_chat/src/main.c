
#include "aroop/aroop_core.h"
#include "aroop/core/xtring.h"
#include "nginz_config.h"
#include "log.h"
#include <signal.h>
#include "plugin_manager.h"
#include "fiber.h"
#include "fork.h"
#include "db.h"
#include "shake/quitall.h"
#include "streamio.h"
#include "chat.h"
#include "event_loop.h"
#include "apps/web_chat/web_chat.h"

C_CAPSULE_START

static int nginz_init() {
#ifdef HAS_MEMCACHED_MODULE
	db_module_init();
#endif
	nginz_core_init();
	nginz_net_init();
#ifdef HAS_CHAT_MODULE
	chat_module_init();
#endif
#ifdef HAS_HTTP_MODULE
	http_module_init();
#endif
#ifdef HAS_WEB_CHAT_MODULE
	web_chat_module_init();
#endif
	nginz_master_init();
	return 0;
}

static int nginz_deinit() {
	tcp_listener_deinit();
#ifdef HAS_WEB_CHAT_MODULE
	web_chat_module_deinit();
#endif
#ifdef HAS_HTTP_MODULE
	http_module_deinit();
#endif
#ifdef HAS_CHAT_MODULE
	chat_module_deinit();
#endif
	protostack_deinit();
	nginz_core_deinit();
#ifdef HAS_MEMCACHED_MODULE
	db_module_deinit();
#endif
}

static int nginz_main(char**args) {
	daemon(0,0);
	setlogmask (LOG_UPTO (LOG_NOTICE));
	openlog ("nginz", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
	nginz_init();
	fiber_module_run();
	nginz_deinit();
	closelog();
	return 0;
}

int main(int argc, char**argv) {
	aroop_main1(argc, argv, nginz_main);
}

C_CAPSULE_END
