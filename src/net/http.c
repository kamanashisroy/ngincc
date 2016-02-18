
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "aroop/opp/opp_factory.h"
#include "aroop/opp/opp_iterator.h"
#include "aroop/opp/opp_factory_profiler.h"
#include "nginz_config.h"
#include "event_loop.h"
#include "plugin.h"
#include "log.h"
#include "plugin_manager.h"
#include "net/protostack.h"
#include "net/http.h"
#include "net/http/http_factory.h"
#include "net/http/http_accept.h"
#include "net/http/http_parser.h"
#include "net/http/http_plugin_manager.h"

C_CAPSULE_START

static struct http_hooks hooks = {};
int http_module_init() {
	memset(&hooks, 0, sizeof(hooks));
	http_factory_module_init();
	http_accept_module_init();
	http_parser_module_init();
	http_plugin_manager_module_init();

	// setup the hooks
	aroop_txt_t plugin_space = {};
	aroop_txt_embeded_set_static_string(&plugin_space, "httpproto/hookup");
	composite_plugin_bridge_call(pm_get(), &plugin_space, HTTP_SIGNATURE, &hooks);
}

int http_module_deinit() {
	http_plugin_manager_module_deinit();
	http_parser_module_deinit();
	http_accept_module_deinit();
	http_factory_module_deinit();
}


C_CAPSULE_END
