#include <functional>
#include <sstream>
#include <stdexcept>
#include <system_error>
#include <cstring> // defines memset
#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <vector>

#include "ngincc_config.h"
#include "log.hxx"
#include "plugin_manager.hxx"
#include "event_loop.hxx"
#include "binary_coder.hxx"
#include "parallel/pipeline.hxx"

using std::string;
using std::istringstream;
using std::ostringstream;
using std::system_error;
using std::logic_error;
using std::function;
using std::vector;
using ngincc::core::event_loop;
using ngincc::core::buffer_coder;
using ngincc::core::plugin_manager;
using ngincc::core::parallel::pipeline;
using namespace std::placeholders;

/****************************************************/
/********* This file is really tricky ***************/
/****************************************************/

pipeline::ngincc_node::ngincc_node() : nid(0) {
}

static int pipeline_already_created = 0;
//! \brief ctor
pipeline::pipeline(plugin_manager& core_plug, event_loop& eloop)
    : masterpid(getpid()),nodes{},mynode(&nodes[0]),eloop(eloop), core_plug(core_plug)
{
    if(pipeline_already_created) {
        throw std::overflow_error("Pipeline cannot be created twice");
    }
	//memset(nodes, 0, sizeof(nodes));
	int i = 0;
	/* make all the pipes beforehand, so that all of the nodes know each other */
	for(i = 0; i < MAX_PROCESS_COUNT; i++) {
		if(socketpair(AF_UNIX, SOCK_DGRAM, 0, nodes[i].fd) || socketpair(AF_UNIX, SOCK_DGRAM, 0, nodes[i].raw_fd)) {
			syslog(LOG_ERR, "Failed to create pipe:%s\n", strerror(errno));
			throw system_error(errno,std::system_category(),"Failed to create pipe");
		}
	}
    // setup mynode
	mynode->nid = getpid();
    std::function<int(int)> callback = std::bind(&pipeline::pp_fork_child_after_callback,this,_1);
    core_plug.plug_add("fork/child/after", "It allows the processes to pipeline messages to and forth.", std::move(callback));
    callback = std::bind(&pipeline::pp_fork_parent_after_callback,this,_1);
    core_plug.plug_add("fork/parent/after", "It allows the processes to pipeline messages to and forth.", std::move(callback));
    std::function<int(buffer_coder& msgbuf)> callback2 = std::bind(&pipeline::pp_update_siblings_pid,this,_1);
    core_plug.plug_add("mailbox/child/pid", "It shares the pid of the siblings to each other.", std::move(callback2));
    std::function<int(vector<string>&, ostringstream&)> shake_callback = std::bind(&pipeline::ping_command,this,_1,_2);
    core_plug.plug_add("shake/ping", "It pings the child process. for example(ping quit) will quit all the child process.", std::move(shake_callback));
    pipeline_already_created = 1;
}

pipeline::~pipeline() {
	// TODO unregister all
}

int pipeline::pp_get_raw_fd(int nid) const {
	int i = 0;
	for(i = 0; i < MAX_PROCESS_COUNT; i++) {
		if(nodes[i].nid == nid)
			return (nodes+i)->raw_fd[0];
	}
	return -1; /* not found */
}

const struct pipeline::ngincc_node*pipeline::pp_find_node(int nid) const {
	int i = 0;
	for(i = 0; i < MAX_PROCESS_COUNT; i++) {
		if(nodes[i].nid == nid)
			return (nodes+i);
	}
	return NULL; /* not found */
}

int pipeline::pp_next_nid() const {
	int i = 0;
	int next_index = -1;
	for(i = 0; i < MAX_PROCESS_COUNT; i++) {
		if((nodes+i) == mynode) {
			next_index = i+1;
			break;
		}
	}
	if(next_index != -1 && next_index < MAX_PROCESS_COUNT)
		return nodes[next_index].nid;
	return -1; /* not found */
}

int pipeline::pp_next_worker_nid(int nid) const {
	int i = 0;
	int next_index = -1;
	for(i = 0; i < MAX_PROCESS_COUNT; i++) {
		if(nodes[i].nid == nid) {
			next_index = i+1;
			break;
		}
	}
	next_index = next_index%MAX_PROCESS_COUNT;
	if(next_index == 0) /* if it is master */
		next_index++; /* take the first childid */
	return nodes[next_index].nid;
}

int pipeline::pp_simple_sendmsg(int through, uint8_t*data, unsigned int data_len) const {
	struct msghdr msg;
	struct iovec iov[1];
	memset(&msg, 0, sizeof(msg));
	memset(iov, 0, sizeof(iov));
	iov[0].iov_base = data;
	iov[0].iov_len  = data_len;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	if(sendmsg(through, &msg, 0) < 0) {
		syslog(LOG_ERR, "Cannot send simple message to child:%s\n", strerror(errno));
		return -1;
	}
	return 0;
}

int pipeline::pp_send(int dnid, uint8_t *msg, unsigned int msg_len) const {
    if(NULL == msg || 0 == msg_len) {
        throw std::underflow_error("No data to send");
    }
	const struct ngincc_node*nd = NULL;
	int i = 0;
	// sanity check
	if(dnid == 0) { /* broadcast */
		for(i = 0; i < MAX_PROCESS_COUNT; i++) {
			if((nodes+i) != mynode && pp_simple_sendmsg(nodes[i].fd[0], msg, msg_len))
				return -1;
		}
	} else {
		nd = pp_find_node(dnid);
        if(NULL == nd || nd->nid != dnid) {
            throw logic_error("destination id retrieval error");
        }
		return pp_simple_sendmsg(nd->fd[0], msg, msg_len);
	}
	return 0;
}

bool pipeline::is_master() const {
	return (mynode && mynode == nodes);
}

/****************************************************/
/********** Pipe event listeners ********************/
/****************************************************/

int pipeline::pp_simple_recvmsg_helper(int through) {
	struct msghdr msg;
	struct iovec iov[1];
	if(through == -1)
		return -1;
	memset(&msg, 0, sizeof(msg));
	memset(iov, 0, sizeof(iov));
    recv_buffer.reset();
    uint8_t* content = recv_buffer.data();
	iov[0].iov_base = content;
	iov[0].iov_len  = recv_buffer.capacity();
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;
	
	int recvlen = 0;
	if((recvlen = recvmsg(through, &msg, 0)) < 0) {
        // set the buffer to be empty
        recv_buffer.reset();
		syslog(LOG_ERR, "Cannot recv msg:%s\n", strerror(errno));
		return -1;
	}
	if(msg.msg_iovlen == 1 && iov[0].iov_len > 0) {
        // update the length of receive buffer
        recv_buffer.set_rd_length(iov[0].iov_len);
		syslog(LOG_NOTICE, "received data:%ld=%ld\n", recv_buffer.in_avail(),iov[0].iov_len);
	}
	return 0;
}


int pipeline::on_bubbles(int fd, int events) {
    if(!mynode || (fd != mynode->fd[1])) {
        throw logic_error("on_bubble: the event-fd mismatch with the pipe-input-fd");
    }
	if(NGINZ_POLL_CLOSE_FLAGS == (events & NGINZ_POLL_CLOSE_FLAGS)) {
		// TODO check if it happens
        // TODO quitall !!! if it is core-pipeline
		syslog(LOG_ERR, "Somehow the pipe is closed\n");
		eloop.unregister_fd(fd);
		close(fd);
		return 0;
	}
	if(pp_simple_recvmsg_helper(fd) || recv_buffer.in_avail() == 0) {
		syslog(LOG_ERR, "Error receiving bubble_down:%s\n", strerror(errno));
		eloop.unregister_fd(fd);
		close(fd);
		return 0;
	}

#if 0
	// TODO use the srcpid for something ..
	//aroop_txt_embeded_rebuild_and_set_content(&recv_buffer, rbuf)
	int srcpid = 0;
	//printf("There is bubble_down from the parent, %d, (count=%d)\n", (int)aroop_txt_char_at(&recv_buffer, 0), count);
	binary_unpack_int(&recv_buffer, 0, &srcpid);
	syslog(LOG_NOTICE, "[pid:%d]\treceiving from parent for %d", getpid(), srcpid);
#endif
    binary_coder coder(recv_buffer);
    string start;
    uint32_t srcpid = 0;
    string service;
    coder >>= start;
    if(start != binary_coder::canary_begin) {
        throw std::underflow_error("on_bubbles:canary check failed");
    }
    coder >>= srcpid;
    coder >>= service;
	if(service.size()) {
		if(core_plug.plug_call<ngincc::core::buffer_coder&>(move(service), {recv_buffer})) {
			syslog(LOG_NOTICE, "[pid:%d]\tplugin returns error\n", getpid());
		}
	}
	//syslog(LOG_NOTICE, "[pid:%d]\texecuting command:%s", getpid(), aroop_txt_to_string(&x));
	return 0;
}

/****************************************************/
/********** Fork event listeners ********************/
/****************************************************/

//#define NGINZ_CLOSE_UNUSED_SOCK
int pipeline::pp_fork_child_after_callback(int child_pid) {
	int i = 0;
	/****************************************************/
	/********* Cleanup old parent fds *******************/
	/****************************************************/
	mynode = NULL;
	for(i = 0; i < MAX_PROCESS_COUNT; i++) {
		if(!mynode && !nodes[i].nid) {
			mynode = (nodes+i);
#ifdef NGINZ_CLOSE_UNUSED_SOCK
			close(nodes[i].fd[0]);
			close(nodes[i].raw_fd[0]);
#endif
			mynode->nid = getpid();
		} else {
#ifdef NGINZ_CLOSE_UNUSED_SOCK
			close(nodes[i].fd[1]);
			close(nodes[i].raw_fd[1]);
#endif
		}
	}
	/****************************************************/
	/********* Register readers *************************/
	/****************************************************/
	eloop.register_fd(mynode->fd[1], std::bind(&pipeline::on_bubbles,this,_1,_2), NGINZ_POLL_ALL_FLAGS);
	core_plug.plug_call<int&>("parallel/pipeline/raw/setup", {std::ref(mynode->raw_fd[1])});
	return 0;
}

int pipeline::pp_fork_parent_after_callback(int child_pid) {
	int i = 0;
	for(i = 0; i < MAX_PROCESS_COUNT; i++) {
		if(!nodes[i].nid) {
			nodes[i].nid = child_pid; /* TODO set child pid */
			break;
		}
	}
	if(getpid() != mynode->nid) {
        throw logic_error("mynode pid mismatch after child creation");
    }
	/* check if the forking is all complete */
	if(nodes[MAX_PROCESS_COUNT-1].nid) { /* if there is no more forking cleanup */
		for(i = 1/* skip the master */; i < MAX_PROCESS_COUNT; i++) {
			/* close the read fd */
#ifdef NGINZ_CLOSE_UNUSED_SOCK
			close(nodes[i].fd[1]);
			close(nodes[i].raw_fd[1]);
#endif
			/* broadcast the pid of the child */
			async_reply_worker(0, nodes[i].nid, "mailbox/child/pid", i, {});
		}
		/****************************************************/
		/********* Register readers *************************/
		/****************************************************/
	    eloop.register_fd(mynode->fd[1], std::bind(&pipeline::on_bubbles,this,_1,_2), NGINZ_POLL_ALL_FLAGS);
	    core_plug.plug_call<int&>("parallel/pipeline/raw/setup", {std::ref(mynode->raw_fd[1])});
	}
	return 0;
}

int pipeline::pp_update_siblings_pid(buffer_coder& msgbuf) {
    string start;
	uint32_t srcpid = 0;
    string hook = 0;
	uint32_t nid = 0;
	uint32_t index = 0;
    binary_coder coder(msgbuf);
    coder >>= start;
    if(start != binary_coder::canary_begin) {
        throw std::underflow_error("on_bubbles:canary check failed");
    }
    coder >>= srcpid;
    coder >>= hook;
    coder >>= nid;
    coder >>= index;
	//binary_unpack_int(input, 2, &nid); // id/token
	//binary_unpack_int(input, 3, &index); // target index
	if((nodes+index) != mynode) {
		nodes[index].nid = nid;
	}
	return 0;
}


