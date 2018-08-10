
#include <functional>
#include <stdexcept>

#include "ngincc_config.h"
#include "module.hxx"
#include "plugin_manager.hxx"
#include "worker.hxx"
#include "lazy_worker.hxx"

using std::string;
using std::function;
using std::overflow_error;
using ngincc::core::worker;
using ngincc::core::lazy_worker;

// std::identity is available in C++20
static std::function<int(int)> identity = [] (int x) {return x;};
static std::function<int()> producer = [] () {return 0;};

lazy_worker::future_task::future_task() : task(producer),when_complete(identity) {
}

lazy_worker::lazy_worker(worker &lazy_worker) : lazy_stack_count(0) {
    int i = NGINZ_LAZY_STACK_SIZE;
    while(i--) {
        lazy_stack[i].task = producer;
        lazy_stack[i].when_complete = identity;
    }
    function<int(int)> step = [this] (int status) {
        int i = lazy_stack_count;
        while(i--) {
            int result = lazy_stack[i].task();
            lazy_stack[i].when_complete(result);
        }
        lazy_stack_count = 0;
        return 0;
    };
    lazy_worker.register_full(step, __FILE__, __LINE__);
}

lazy_worker::~lazy_worker() {
    // TODO unregister_fiber(step)
}

int lazy_worker::add_future_task(std::function<int()>& task) {
    add_future_task(task,identity);
    return 0;
}

int lazy_worker::add_future_task(std::function<int()> &task,std::function<int(int)> &when_complete) {
	if(lazy_stack_count >= NGINZ_LAZY_STACK_SIZE) {
        throw std::overflow_error("too many lazy tasks");
    }
	//lazy_stack[lazy_stack_count] = lazy_worker::future_task(task,when_complete);
	lazy_stack[lazy_stack_count].task = task;
	lazy_stack[lazy_stack_count].when_complete = when_complete;
	lazy_stack_count++;
    return 0;
}

