#include <functional>
#include <sstream>
#include <vector>
#include <unistd.h>

#include "module.hxx"
#include "parallel/pipeline.hxx"

using std::string;
using std::ostringstream;
using std::endl;
using std::vector;
using ngincc::core::parallel::pipeline;
using ngincc::core::binary_coder;

int pipeline::ping_command(vector<string> &command_args, ostringstream &output) {

    int dstpid = 0;
    if(command_args.size() > 2) {
        dstpid = std::stoi(command_args[1]);
    }
    if(0 == dstpid) {
        output << "No destination pid is given" << endl;
        return 0;
    }
    send_buffer.reset();
    binary_coder coder(send_buffer);
    coder <<= binary_coder::canary_begin;
    coder <<= getpid();
    bool first = true;
    for(auto x : command_args) {
        if(first) {
            first = false;
            continue; // skip first
        }
        coder <<= x;
    }
    coder <<= binary_coder::canary_end;

    uint8_t* content = send_buffer.data();

    if(pp_send(dstpid, content, send_buffer.out_avail())) {
		output << " ping failed " << endl;
	} else {
        output << "ping " << endl;
    }
	return 0;
}


