
#include <aroop/aroop_core.h>
#include <aroop/core/xtring.h>
#include <sys/socket.h>
#include "nginz_config.h"
#include "event_loop.h"
#include "plugin.h"


#include "log.hxx"
#include "plugin_manager.h"
#include "protostack.h"
#include "streamio.h"
#include "binary_coder.h"
#include "raw_pipeline.h"


int default_net_channel::send(string& content, int flag) {
	if(INVALID_FD == fd) {
		syslog(LOG_ERR, "There is a dead chat\n");
		return -1;
	}
	return send(fd, content.c_str(), content.size(), flag);
}

int default_net_channel::send_nonblock(string& content, int flag) {
	if(INVALID_FD == fd) {
		syslog(LOG_ERR, "There is a dead chat\n");
		return -1;
	}
	int err = send(fd, content.c_str(), content.size(), flag /*| MSG_DONTWAIT*/); // TODO use nonblocking io
	if(err == -1) {
		if((errno == EWOULDBLOCK || errno == EAGAIN)) {
			// TODO implement nonblocking io
			syslog(LOG_ERR, "Could not write network data asynchronously ..");
			error = errno;
			return -1;
		}
		syslog(LOG_ERR, "closing socket because while streaming :%s", strerror(errno));
		error = errno;
		//close_handle();
		return err;
	}
	return err;
}

int default_net_channel::close_handle(bool soft) {
	if(INVALID_FD != fd) {
		eloop.unregister_fd(fd);
		close(fd);
		fd = INVALID_FD;
	}
	return 0;
}

int default_net_channel::transfer_parallel(struct streamio*strm, int destpid, int proto_port, aroop_txt_t*cmd) {
	//syslog(LOG_NOTICE, "default_transfer_parallel: transfering to %d", destpid);
	if(strm->bubble_up) {
		//syslog(LOG_NOTICE, "default_transfer_parallel: bubble up ");
		return strm->bubble_up->transfer_parallel(strm->bubble_up, destpid, proto_port, cmd);
	}
	aroop_txt_t bin = {};
	aroop_txt_embeded_stackbuffer(&bin, 255);
	binary_coder_reset_for_pid(&bin, destpid);
	binary_pack_int(&bin, proto_port);
	binary_pack_string(&bin, cmd);

	event_loop_unregister_fd(strm->fd);
	return pp_raw_send_socket(destpid, strm->fd, &bin);
}

default_net_channel::default_net_channel(
    int fd
    : fd(fd)
    , error(0){
}

default_net_channel::~default_net_channel() {
    close_handle();
}


