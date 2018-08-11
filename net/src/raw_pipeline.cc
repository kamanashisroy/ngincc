
#include <vector>
#include <iostream>
#include <sstream>
#include <cstring> // defines memset
#include <unistd.h>
#include <sys/socket.h>

#include "log.hxx"
#include "module.hxx"
#include "plugin_manager.hxx"
#include "event_loop.hxx"
#include "tcp_listener_server.hxx"
#include "parallel/pipeline.hxx"
#include "binary_coder.hxx"
#include "raw_pipeline.hxx"

using std::string;
using std::ostringstream;
using std::endl;
using std::vector;
using ngincc::core::plugin_manager;
using ngincc::core::event_loop;
using ngincc::core::parallel::pipeline;
using ngincc::net::raw_pipeline;
using ngincc::net::server_stack;
using ngincc::core::buffer_coder;
using ngincc::core::binary_coder;
using namespace std::placeholders;
using namespace ngincc::net;

int raw_pipeline::sendmsg_helper(int through, int target, string&cmd) {
	union {
		int target;
		char buf[CMSG_SPACE(sizeof(int))];
	} intbuf;
	intbuf.target = target;
	struct msghdr msg;
	struct iovec iov[1];
	struct cmsghdr *control_message = NULL;
	memset(&intbuf, 0, sizeof(intbuf));
	// sanity check
	if(through == -1)
		return -1;
	memset(&msg, 0, sizeof(msg));
	memset(iov, 0, sizeof(iov));

	iov[0].iov_base = (void*)cmd.c_str();
	iov[0].iov_len  = cmd.size();
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	msg.msg_control = intbuf.buf;
	msg.msg_controllen = CMSG_SPACE(sizeof(int));
	control_message = CMSG_FIRSTHDR(&msg);
	control_message->cmsg_level = SOL_SOCKET;
	control_message->cmsg_type = SCM_RIGHTS;
	control_message->cmsg_len = CMSG_LEN(sizeof(int));
	*((int *) CMSG_DATA(control_message)) = target;
	//printf("Sending fd %d to worker\n", target);
	
	if(sendmsg(through, &msg, 0) < 0) {
		syslog(LOG_ERR, "Cannot send message to child:%s\n", strerror(errno));
		return -1;
	}
	close(target); // we do not own this fd anymore
	return 0;
}

int raw_pipeline::send_socket(int destpid, int socket, string& cmd) {
	int fd = pipe.pp_get_raw_fd(destpid);
	if(fd == -1) {
		syslog(LOG_ERR, "We could not find raw fd for:%d\n", destpid);
		return -1;
	}

	return sendmsg_helper(fd, socket, cmd);
}

/****************************************************/
/********** Pipe event listeners ********************/
/****************************************************/

int raw_pipeline::recvmsg_helper(int through, int&target) {
	//printf("There is new client, we need to accept it in worker process\n");
	union {
		int target;
		char buf[CMSG_SPACE(sizeof(int))];
	} intbuf;
	struct msghdr msg;
	struct iovec iov[1];
	struct cmsghdr *control_message = NULL;
	memset(&intbuf, 0, sizeof(intbuf));
	// sanity check
	if(through == -1)
		return -1;
	memset(&msg, 0, sizeof(msg));
	memset(iov, 0, sizeof(iov));

    recv_buffer.reset();
	iov[0].iov_base = recv_buffer.data();
	iov[0].iov_len  = recv_buffer.capacity();
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	msg.msg_control = intbuf.buf;
	msg.msg_controllen = CMSG_SPACE(sizeof(int));
	
	int recvlen = 0;
	if((recvlen = recvmsg(through, &msg, 0)) < 0) {
		syslog(LOG_ERR, "Cannot recv msg:%s\n", strerror(errno));
		return -1;
	}
	if(msg.msg_iovlen == 1 && iov[0].iov_len > 0) {
		recv_buffer.set_rd_length(iov[0].iov_len);
	}
	for(control_message = CMSG_FIRSTHDR(&msg);
		control_message != NULL;
		control_message = CMSG_NXTHDR(&msg,
				   control_message)) {
		if( (control_message->cmsg_level == SOL_SOCKET) &&
		(control_message->cmsg_type == SCM_RIGHTS) )
		{
			target = *((int *) CMSG_DATA(control_message));
		}
	}
	return 0;
}

int raw_pipeline::on_raw_recv_socket(int fd, int events) {
	uint32_t port = 0;
	uint32_t srcpid = 0;
	int acceptfd = -1;
    syslog(LOG_NOTICE, "[pid:%d]\treceiving from parent", getpid());
    if(recvmsg_helper(fd, acceptfd)) {
        return 0;
    }
    binary_coder coder(recv_buffer);
    coder >>= srcpid;
    coder >>= port;
    for(auto&& stack : tcp_server_list) {
        // this is costly O(n) code
        if(port == (uint32_t)stack.get_port()) {
            stack.on_connection_bubble(acceptfd);
            break;
        }
    }
	return 0;
}

int raw_pipeline::on_raw_socket_setup(int raw_fd) {
    std::function<int(int,int)> callback = std::bind(&raw_pipeline::on_raw_recv_socket,this,_1,_2);
	eloop.register_fd(raw_fd, move(callback), NGINZ_POLL_ALL_FLAGS);
	return 0;
}

raw_pipeline::raw_pipeline(
        plugin_manager& net_plugs
        , event_loop& eloop
        , pipeline& pipe
        , vector<server_stack>& tcp_server_list
    ) :
    recv_buffer()
    , eloop(eloop)
    , pipe(pipe)
    , tcp_server_list(tcp_server_list) {
    std::function<int(int)> callback = std::bind(&raw_pipeline::on_raw_socket_setup,this,_1);
	net_plugs.plug_add("parallel/pipeline/raw/setup"
        , "It registers event listener for raw socket."
        , move(callback));
}

raw_pipeline::~raw_pipeline() {
	// TODO pm_unplug_bridge(0, on_raw_socket_setup);
}

