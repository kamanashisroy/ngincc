
#include <functional>
#include <vector>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <system_error>
#include <unistd.h>
#include <cstring> // defines strerror
#include <cerrno>

#include "log.hxx"
#include "plugin_manager.hxx"
#include "event_loop.hxx"
#include "worker.hxx"

using std::string;
using std::ostringstream;
using std::endl;
using std::vector;
using std::overflow_error;
using std::system_error;
using ngincc::core::event_loop;
using ngincc::core::worker;
using ngincc::core::plugin_manager;
using ngincc::core::event_loop;
using ngincc::core::event_loop_poll;

int event_loop_poll::count() const {
	return internal_nfds;
}

int event_loop_poll::register_fd(int fd, std::function<int(int,int)>&& callback, int requested_events) {
    if(internal_nfds >= MAX_POLL_FD) {
        throw overflow_error("maximum fd limit exceeded in event-loop");
    }
	internal_fds[internal_nfds].fd = fd;
	internal_fds[internal_nfds].events = requested_events;
	internal_fds[internal_nfds].revents = 0; // make sure we continue the event_loop without any conflict
	internal_callback[internal_nfds].on_event = callback;
	internal_nfds++;
	return 0;
}

#ifdef NGINZ_EVENT_DEBUG
int event_loop_register_debug(int fd, int (*on_debug)(int fd, const void*debug_data)) {
	int i = 0;
	for(i = 0; i < internal_nfds; i++) {
		if(internal_fds[i].fd == fd) {
			internal_callback[i].on_debug = on_debug;
		}
	}
}

static int event_loop_debug() {
	int i = 0;
	for(i = 0; i < internal_nfds; i++) {
		if(internal_fds[i].events && internal_callback[i].on_debug)
			internal_callback[i].on_debug(internal_fds[i].fd, internal_callback[i].event_data);
	}
}
#endif

int event_loop_poll::unregister_fd(const int fd) {
	int i = 0;
	const int active_nfds = internal_nfds;
	for(i = 0; i < active_nfds; i++) {
		if(internal_fds[i].fd != fd)
			continue;
		if(!internal_fds[i].events)
			continue;
		internal_fds[i].events = 0; // lazy unregister
		break;
	}
	return 0;
}

int event_loop_poll::batch_unregister() {
	int i = 0;
	int last = internal_nfds - 1;
	// trim the last invalid elements
	for(; last >= 0 && !internal_fds[last].events; last--);
	// now we swap the invalid with last
	for(i=0; i < last; i++) {
		if(internal_fds[i].events)
			continue;
		internal_fds[i] = internal_fds[last];
		internal_callback[i] = internal_callback[last];
		last--;
		for(; last >= 0 && !internal_fds[last].events; last--);
	}
	internal_nfds = last+1;
	return 0;
}

int event_loop_poll::step_helper(int count) {
	int i = 0;
	for(i = 0; i < internal_nfds && count; i++) {
		if(!internal_fds[i].events || !internal_fds[i].revents) {
			continue;
		}
		count--;
		internal_callback[i].on_event(internal_fds[i].fd, internal_fds[i].revents);
	}
	batch_unregister();
	return 0;
}

int event_loop_poll::test_helper(int test_count) {
	int fd = 5000;
	int ncount = test_count;
	int prev = count();
    std::function<int(int,int)> no_action;
	while(ncount--) {
		register_fd(fd+ncount, move(no_action), POLLIN);
	}
	ncount = test_count;
	while(ncount--) {
		unregister_fd(fd+ncount);
	}
	batch_unregister();
	
	return !(prev == count());
}

event_loop_poll::event_loop_poll(ngincc::core::worker &event_worker, ngincc::core::plugin_manager &pm) {
	event_worker.register_full([this](int status) {
        if(internal_nfds == 0) {
            usleep(1000);
            return 0;
        }
        int count = 0;
        if(!(count = poll(internal_fds, internal_nfds, 100)))
            return 0;
        if(count == -1) {
            syslog(LOG_ERR, "event_loop.c poll failed %s", strerror(errno));
            //throw system_error(errno,std::generic_category,move(string("poll failed")));
        }
        return step_helper(count);
    },__FILE__, __LINE__);
    std::function<int(vector<string>&, ostringstream&)> test_callback = [this](vector<string>&unused, std::ostringstream &output) {
        if(test_helper(1000)) {
            output << "event_loop.c:FAILED" << endl;
        } else {
            output << "event_loop.c:successful" << endl;
        }
        return 0;
    };
    pm.plug_add("test/event_loop_test", "It is test code for event loop.", move(test_callback));
}

event_loop_poll::~event_loop_poll() {
    //TODO event_fiber.unregister(fiberid);
	// TODO pm_unplug_callback(0, event_loop_test);
}

event_loop::~event_loop() {
}

