
#include <functional>
#include <string>
#include <sstream>
#include <iostream>

#include "ngincc_config.h"
#include "log.hxx"
#include "base_subsystem.hxx"
#include "net_subsystem.hxx"
#include "chat_subsystem.hxx"


using ngincc::apps::chat::chat_subsystem;

static int run_chat() {
    chat_subsystem chat;
    chat.parallel_init();
    chat.run();
    return 0;
}

int main(int argc, char**argv) {
	//daemon(0,0);
	setlogmask (LOG_UPTO (LOG_NOTICE));
	openlog ("nginz_chat", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    run_chat();
	closelog();
	return 0;
}


/*static int nginz_main(char**args) {
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
}*/

