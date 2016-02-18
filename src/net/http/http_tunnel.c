
#include <sys/socket.h>
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
#include "net/http/http_tunnel.h"

C_CAPSULE_START

static aroop_txt_t send_buf = {};
static aroop_txt_t HTTP_OK = {};
int http_tunnel_send_content(struct http_connection*http, aroop_txt_t*content) {
	aroop_txt_set_length(&send_buf, 0);
	aroop_txt_printf(&send_buf, "%d\r\n\r\n", aroop_txt_length(content));
	
	send(http->fd, aroop_txt_to_string(&HTTP_OK), aroop_txt_length(&HTTP_OK), MSG_MORE/* we have more data */);
	send(http->fd, aroop_txt_to_string(&send_buf), aroop_txt_length(&send_buf), MSG_MORE/* we have more data */);
	send(http->fd, aroop_txt_to_string(content), aroop_txt_length(content), 0/* we have are done */);
	http->is_processed = 1;
	return 0;
}

int http_tunnel_module_init() {
	aroop_txt_embeded_buffer(&send_buf, NGINZ_MAX_HTTP_MSG_SIZE);
	aroop_txt_embeded_set_static_string(&HTTP_OK, "HTTP/1.0 200 OK\r\nContent-Length: ");
}

int http_tunnel_module_deinit() {
	aroop_txt_destroy(&HTTP_OK);
	aroop_txt_destroy(&send_buf);
}

C_CAPSULE_END
