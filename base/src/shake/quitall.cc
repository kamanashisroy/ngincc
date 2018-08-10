
#include <functional>
#include <vector>
#include <iostream>
#include <sstream>
#include <unistd.h> // defines getpid()

#include "plugin_manager.hxx"
#include "worker.hxx"
#include "shake/shake_intrprtr.hxx"
#include "shake/quitall_internal.hxx"
#include "binary_coder.hxx"
#include "parallel/pipeline.hxx"

using std::string;
using std::ostringstream;
using std::endl;
using std::vector;
using ngincc::core::worker;
using ngincc::core::buffer_coder;
using ngincc::core::binary_coder;
using ngincc::core::plugin_manager;
using ngincc::core::parallel::pipeline;
using namespace ngincc::core::shake;

static int soft_quitall = 0;

quitall::quitall(plugin_manager&core_plug,worker&core_worker,pipeline&pipe) : core_worker(core_worker),pipe(pipe) {
    
    std::function<int(vector<string>&, ostringstream&)> callback = [this] (vector<string> &cmd_args, ostringstream &output) {
        if(soft_quitall) /* soft quit is already complete */
            return 0;
        int next = this->pipe.pp_next_nid();
        if(next != -1) { // propagate the command
            output << "Sending softquit to all" << endl;
            buffer_coder buffer;
            binary_coder coder(buffer);
            coder <<= getpid();
            coder <<= string("shake/softquitall");
            this->pipe.pp_send(next, buffer.data(), buffer.in_avail());
        }
        soft_quitall = 1;
        return 0;
    };
    core_plug.plug_add(
        "shake/softquitall"
        ,"It stops the fibers and sends quitall msg to other processes"
        ,move(callback));
    callback = [this] (vector<string> &cmd_args, ostringstream &output) {
        if(!soft_quitall) {
            output << "Please softquitall first" << endl;
            return 0;
        }
        int next = this->pipe.pp_next_nid();
        if(next != -1) { // propagate the command
            output << "Sending QUIT to all" << endl;
            buffer_coder buffer;
            binary_coder coder(buffer);
            coder <<= getpid();
            coder <<= string("shake/quitall");
            this->pipe.pp_send(next, buffer.data(), buffer.in_avail());
        }
        this->core_worker.quit();
        return 0;
    };
    core_plug.plug_add(
        "shake/quitall"
        ,"It stops the fibers and sends quitall msg to other processes"
        ,move(callback)
    );
}


quitall::~quitall() {
    // TODO unplug
	//pm_unplug_callback(0, shake_quitall_command);
	//pm_unplug_callback(0, shake_soft_quitall_command);
}


