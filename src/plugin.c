
#include "aroop/aroop_core.h"
#include "aroop/core/thread.h"
#include "aroop/core/xtring.h"
#include "aroop/opp/opp_factory.h"
#include "aroop/opp/opp_iterator.h"
#include "aroop/opp/opp_factory_profiler.h"
#include "aroop/opp/opp_any_obj.h"
#include "aroop/opp/opp_str2.h"
#include "aroop/aroop_memory_profiler.h"
#include "plugin.h"

C_CAPSULE_START

enum plugin_category {
	CALLBACK_PLUGIN = 0,
	COMPOSITE_PLUGIN,
	BRIDGE_PLUGIN,
};

struct composite_plugin { 
	struct opp_factory factory;
	opp_hash_table_t table;
};


struct internal_plugin {
	int category;
	aroop_txt_t*plugin_space;
	union {
		int (*callback)(aroop_txt_t*input, aroop_txt_t*output);
		int (*bridge)(int signature, void*x);
		struct composite_plugin*inner;
	} x;
	int(*desc)(aroop_txt_t*plugin_space, aroop_txt_t*output);
	struct internal_plugin*next;
};

static struct opp_factory cplugs;
struct composite_plugin*composite_plugin_create() {
	return OPP_ALLOC1(&cplugs);
}

OPP_CB(internal_plugin) {
	struct internal_plugin*plugin = data;
	switch(callback) {
		case OPPN_ACTION_INITIALIZE:
			plugin->category = 0;
			plugin->plugin_space = NULL;
			plugin->desc = NULL;
			plugin->next = NULL;
		break;
		case OPPN_ACTION_FINALIZE:
			OPPUNREF(plugin->plugin_space);
			OPPUNREF(plugin->next);
		break;
	}
	return 0;
}

OPP_CB(composite_plugin) {
	struct composite_plugin*cplug = data;
	switch(callback) {
		case OPPN_ACTION_INITIALIZE:
			OPP_PFACTORY_CREATE(&cplug->factory, 2, sizeof(struct internal_plugin), OPP_CB_FUNC(internal_plugin));
			opp_hash_table_create(&cplug->table, 2, 0, aroop_txt_get_hash_cb, aroop_txt_equals_cb);
		break;
		case OPPN_ACTION_FINALIZE:
			OPP_PFACTORY_DESTROY(&cplug->factory);
			opp_hash_table_destroy(&cplug->table);
			
		break;
	}
	return 0;
}

int plugin_init() {
	OPP_PFACTORY_CREATE(&cplugs, 4, sizeof(struct composite_plugin), OPP_CB_FUNC(composite_plugin));
}

int plugin_deinit() {
	OPP_PFACTORY_DESTROY(&cplugs);
}

static int composite_plug_helper(struct composite_plugin*container
	, aroop_txt_t*plugin_space
	, int (*callback)(aroop_txt_t*input, aroop_txt_t*output)
	, int (*bridge)(int signature, void*x)
	, struct composite_plugin*inner
	, int(*desc)(aroop_txt_t*plugin_space, aroop_txt_t*output)
	) {
	struct internal_plugin*plugin = OPP_ALLOC1(&container->factory);
	aroop_assert(plugin != NULL);
	if(callback != NULL) {
		plugin->category = CALLBACK_PLUGIN;
		plugin->x.callback = callback;
	} else if(bridge != NULL) {
		plugin->category = BRIDGE_PLUGIN;
		plugin->x.bridge = bridge;
	} else if(inner != NULL){
		plugin->category = COMPOSITE_PLUGIN;
		plugin->x.inner = inner;
	} else {
		aroop_assert(!"Unrecognised plugin\n");
	}
	plugin->desc = desc;
	plugin->plugin_space = aroop_txt_new_copy_deep(plugin_space, NULL);
	aroop_assert(plugin->plugin_space != NULL);
	aroop_txt_zero_terminate(plugin->plugin_space);
	struct internal_plugin*root = opp_hash_table_get(&(container->table), plugin->plugin_space);
	if(root) {
		OPPREF(plugin);
		root->next = plugin;
	} else {
		opp_hash_table_set(&(container->table), plugin->plugin_space, plugin);
	}
	int ret = 0; // XXX TOKEN DOES NOT WORK
	OPPUNREF(plugin); // cleanup : unref the plugin, it is already saved in the hashtable
	return ret;
}


int composite_plug_callback(struct composite_plugin*container, aroop_txt_t*plugin_space, int (*callback)(aroop_txt_t*input, aroop_txt_t*output), int(*desc)(aroop_txt_t*plugin_space, aroop_txt_t*output)) {
	composite_plug_helper(container, plugin_space, callback, NULL, NULL, desc);
}

int composite_unplug_callback(struct composite_plugin*container, int plugin_id, int (*callback)(aroop_txt_t*input, aroop_txt_t*output)) {
	// TODO FILLME
}

int composite_plug_bridge(struct composite_plugin*container, aroop_txt_t*plugin_space, int (*bridge)(int signature, void*x), int(*desc)(aroop_txt_t*plugin_space, aroop_txt_t*output)) {
	composite_plug_helper(container, plugin_space, NULL, bridge, NULL, desc);
}

int composite_unplug_bridge(struct composite_plugin*container, int plugin_id, int (*bridge)(int signature, void*x)) {
	// TODO FILLME
}

int composite_plug_inner_composite(struct composite_plugin*container, aroop_txt_t*plugin_space, struct composite_plugin*inner, int(*desc)(aroop_txt_t*plugin_space, aroop_txt_t*output)) {
	composite_plug_helper(container, plugin_space, NULL, NULL, inner, desc);
}

int composite_unplug_inner_composite(struct composite_plugin*container, int plugin_id, struct composite_plugin*inner) {
	// TODO FILLME
}

int composite_plugin_call(struct composite_plugin*container, aroop_txt_t*plugin_space, aroop_txt_t*input, aroop_txt_t*output) {
	struct internal_plugin*plugin = opp_hash_table_get(&container->table, plugin_space);
	int ret = 0;
	while(plugin) {
		aroop_assert(plugin->category == CALLBACK_PLUGIN);
		ret |= plugin->x.callback(input, output);
		plugin = plugin->next;
	}
	return ret;
}


int composite_plugin_bridge_call(struct composite_plugin*container, aroop_txt_t*plugin_space, int signature, void*data) {
	struct internal_plugin*plugin = opp_hash_table_get(&container->table, plugin_space);
	int ret = 0;
	while(plugin) {
		aroop_assert(plugin->category == BRIDGE_PLUGIN);
		ret |= plugin->x.bridge(signature, data);
		plugin = plugin->next;
	}
	return ret;
}


int composite_plugin_visit_all(struct composite_plugin*container, int (*visitor)(
		int category
		, aroop_txt_t*plugin_space
		, int(*callback)(aroop_txt_t*input, aroop_txt_t*output)
		, int(*bridge)(int signature, void*x)
		, struct composite_plugin*inner
		, int(*desc)(aroop_txt_t*plugin_space, aroop_txt_t*output)
		, void*visitor_data
	), void*visitor_data) {
	struct opp_iterator iterator;
	opp_iterator_create(&iterator, &container->factory, OPPN_ALL, 0, 0);
	do {
		struct internal_plugin*plugin = opp_iterator_next(&iterator);
		if(plugin == NULL)
			break;
		do {
			visitor(
				plugin->category
				, plugin->plugin_space
				, plugin->x.callback
				, plugin->x.bridge
				, plugin->x.inner
				, plugin->desc
				, visitor_data
			);
		} while((plugin = plugin->next));
		//OPPUNREF(plugin);
	} while(1);
	opp_iterator_destroy(&iterator);
	return 0;
}

int plugin_desc(aroop_txt_t*output, char*plugin_name, char*plugin_type, aroop_txt_t*space, char*source_file, char*desc) {
	aroop_txt_embeded_buffer(output, 128);
	aroop_txt_concat_string(output, plugin_name);
	aroop_txt_concat_char(output, '\t');
	aroop_txt_concat_string(output, plugin_type);
	aroop_txt_concat_char(output, '\t');
	aroop_txt_concat(output, space);
	aroop_txt_concat_char(output, '\t');
	aroop_txt_concat_string(output, source_file);
	aroop_txt_concat_char(output, '\n');
	aroop_txt_concat_string(output, desc);
	return 0;
}


C_CAPSULE_END
