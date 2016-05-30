
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

static int test_command_tester(
	int category
	, aroop_txt_t*plugin_space
	, int(*callback)(aroop_txt_t*input, aroop_txt_t*output)
	, int(*bridge)(int signature, void*x)
	, struct composite_plugin*inner
	, int(*desc)(aroop_txt_t*plugin_space, aroop_txt_t*output)
	, void*visitor_data
) {
	aroop_txt_t prefix = {};
	aroop_txt_t input = {};
	aroop_txt_t*output = (aroop_txt_t*)visitor_data;
	aroop_txt_t xdesc = {};
	aroop_assert(category != INVALID_PLUGIN);
	aroop_assert(!aroop_txt_is_empty_magical(plugin_space));
	aroop_txt_embeded_copy_on_demand(&prefix, plugin_space); // needs cleanup
	aroop_txt_set_length(&prefix, 5);
	do {
		if(!aroop_txt_equals_static(&prefix, "test/"))
			break;
		callback(&input, &xdesc); // needs cleanup
		aroop_txt_concat(output, &xdesc);
	} while(0);
	aroop_txt_destroy(&prefix); // cleanup
	aroop_txt_destroy(&xdesc); // cleanup
	return 0;
}

static int test_command(aroop_txt_t*input, aroop_txt_t*output) {
	composite_plugin_test(pm_get());
	aroop_txt_embeded_buffer(output, 512);
	composite_plugin_visit_all(pm_get(), test_command_tester, output);
	aroop_txt_concat_string(output, "\nTests Complete\n");
	return 0;
}

static int test_command_desc(aroop_txt_t*plugin_space, aroop_txt_t*output) {
	return plugin_desc(output, "test", "shake", plugin_space, __FILE__, "It tests all the test cases.\n");
}

int test_module_init() {
	aroop_txt_t test_plug;
	aroop_txt_embeded_set_static_string(&test_plug, "shake/test");
	pm_plug_callback(&test_plug, test_command, test_command_desc);
	return 0;
}

int test_module_deinit() {
	pm_unplug_callback(0, test_command);
	return 0;
}

C_CAPSULE_END
