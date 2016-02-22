
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include "nginz_config.h"
#include "event_loop.h"
#include "plugin.h"
#include "log.h"
#include "plugin_manager.h"
#include "net/protostack.h"
#include "net/streamio.h"

C_CAPSULE_START


int default_streamio_send(struct streamio*strm, aroop_txt_t*content, int flag) {
	if(strm->bubble_up)
		return strm->bubble_up->send(strm->bubble_up, content, flag);
	aroop_assert(strm->fd != INVALID_FD);
	return send(strm->fd, aroop_txt_to_string(content), aroop_txt_length(content), flag);
}

int default_streamio_close(struct streamio*strm) {
	if(strm->bubble_up) {
		return strm->bubble_up->close(strm->bubble_up);
	}
	strm->bubble_up = NULL;
	if(strm->fd != INVALID_FD) {
		event_loop_unregister_fd(strm->fd);
		close(strm->fd);
		strm->fd = INVALID_FD;
	}
	return 0;
}

int default_transfer_parallel(struct streamio*strm, int destpid, int proto_port, aroop_txt_t*cmd) {
	syslog(LOG_NOTICE, "default_transfer_parallel: transfering to %d", destpid);
	if(strm->bubble_up) {
		syslog(LOG_NOTICE, "default_transfer_parallel: bubble up ");
		return strm->bubble_up->transfer_parallel(strm->bubble_up, destpid, proto_port, cmd);
	}
	int mypid = getpid(); // we may do little optimiztion here by saving getpid() value ..
	aroop_txt_t bin = {};
	aroop_txt_embeded_stackbuffer(&bin, 255);
	binary_coder_reset_for_pid(&bin, destpid);
	binary_pack_int(&bin, proto_port);
	binary_pack_string(&bin, cmd);

	event_loop_unregister_fd(strm->fd);
	if(destpid > mypid) {
		return pp_bubble_down_send_socket(strm->fd, &bin);
	} else {
		return pp_bubble_up_send_socket(strm->fd, &bin);
	}
}

int streamio_initialize(struct streamio*strm) {
	strm->fd = -1;
	strm->on_recv = NULL;
	strm->send = default_streamio_send;
	strm->close = default_streamio_close;
	strm->transfer_parallel = default_transfer_parallel;
	strm->bubble_up = NULL;
	strm->bubble_down = NULL;
	return 0;
}

int streamio_finalize(struct streamio*strm) {
	strm->close(strm);
	strm->fd = -1;
	strm->bubble_up = NULL;
	if(strm->bubble_down)OPPUNREF(strm->bubble_down);
	return 0;
}

int streamio_chain(struct streamio*up, struct streamio*down) {
	if(down->bubble_up) {
		down->close(down);
	}
	down->bubble_up = up;
	if(up->bubble_down) {
		up->bubble_down->close(up->bubble_down);
		OPPUNREF(up->bubble_down);
	}
	up->bubble_down = OPPREF(down);
}

int streamio_unchain(struct streamio*up, struct streamio*down) {
	down->bubble_up = NULL;
	if(up->bubble_down == down) {
		OPPUNREF(up->bubble_down);
	}
	return 0;
}



C_CAPSULE_END

