
#include <unistd.h>
#include "aroop/aroop_core.h"
#include "aroop/core/xtring.h"
#include "nginz_config.h"
#include "log.h"
#include <signal.h>
#include "plugin_manager.h"
#include "event_loop.h"
#include "base_subsystem.h"
#include "net_subsystem.h"
#include "db_subsystem.h"
#include "chat_subsystem.h"
#include "fiber.h"

C_CAPSULE_START

static int nginz_main(char**args) {
	daemon(0,0);
	setlogmask (LOG_UPTO (LOG_NOTICE));
	openlog ("nginz_chat", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
	nginz_core_init();
	nginz_net_init();
	nginz_db_module_init_before_parallel_init();
	nginz_chat_module_init();
	nginz_parallel_init();
	nginz_net_init_after_parallel_init();
	nginz_db_module_init_after_parallel_init();
	fiber_module_run();
	nginz_chat_module_deinit();
	nginz_db_module_deinit();
	nginz_net_deinit();
	nginz_core_deinit();
	closelog();
	return 0;
}

int main(int argc, char**argv) {
	return aroop_main1(argc, argv, nginz_main);
}

C_CAPSULE_END
