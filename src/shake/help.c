
#include "aroop/aroop_core.h"
#include "aroop/core/thread.h"
#include "aroop/core/xtring.h"
#include "aroop/opp/opp_factory.h"
#include "aroop/opp/opp_factory_profiler.h"
#include "aroop/opp/opp_any_obj.h"
#include "aroop/opp/opp_str2.h"
#include "aroop/aroop_memory_profiler.h"
#include "plugin.h"
#include "plugin_manager.h"
#include "shake.h"

C_CAPSULE_START

static int help_command_helper(
	int category
	, aroop_txt_t*plugin_space
	, int(*callback)(aroop_txt_t*input, aroop_txt_t*output)
	, int(*bridge)(int signature, void*x)
	, struct composite_plugin*inner
	, int(*desc)(aroop_txt_t*output)
	, void*visitor_data
) {
	aroop_txt_t*output = (aroop_txt_t*)visitor_data;
	aroop_txt_t xdesc;
	memset(&xdesc, 0, sizeof(xdesc));
	desc(&xdesc);
	aroop_txt_concat(output, &xdesc);
}

static int help_command(aroop_txt_t*input, aroop_txt_t*output) {
	aroop_txt_embeded_buffer(output, 512);
	composite_plugin_visit_all(pm_get(), help_command_helper, output);
}

static int help_command_desc(aroop_txt_t*output) {
	aroop_txt_embeded_rebuild_and_set_static_string(output,
		"Help\n"
		"help command will show the description of the command\n"
	);
}

int help_module_init() {
	aroop_txt_t help_plug;
	aroop_txt_embeded_set_static_string(&help_plug, "shake/help");
	pm_plug_callback(&help_plug, help_command, help_command_desc);
}

int help_module_deinit() {
	pm_unplug_callback(0, help_command);
}

C_CAPSULE_END
