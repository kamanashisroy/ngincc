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
using std::ostringstream;
using std::system_error;
using std::initializer_list;
using ngincc::core::buffer_coder;
using ngincc::core::parallel::pipeline;

//#define DB_LOG(...) syslog(__VA_ARGS__)
#define DB_LOG(...) 


int pipeline::async_request(
    uint32_t dstpid, uint32_t token, const string &reply_hook, const string &request_hook, const initializer_list<std::reference_wrapper<const string> >&& more)
{
    send_buffer.reset();
    binary_coder coder(send_buffer);
    coder <<= binary_coder::canary_begin;
	// send request
	// 0 = src pid, 1 = request_hook, 2 = token, 3 = reply_hook, 4 = key, 5 = newval, 6 = oldval
    coder <<= getpid();                 // send source pid
    coder <<= request_hook;             // master/request_hook
    coder <<= token;                    // token/request-id
    coder <<= reply_hook;               // reply_hook/callback_hook
	for(auto& x : more) {
        coder <<= x;
	}
    coder <<= binary_coder::canary_end;
    uint8_t* content = send_buffer.data();
    auto content_len = send_buffer.out_avail();
	DB_LOG(LOG_NOTICE, "[token%d]-request-[master:%d]-[bytes:%d][app:%s]{%X,%X}", token, masterpid, content_len, request_hook.c_str(), content[0], content[1]);
    pp_send(dstpid, content, content_len);
	return 0;
}

int pipeline::async_request_master(
    uint32_t token, const string &reply_hook, const string &request_hook, const initializer_list< std::reference_wrapper< const string> >&& more
) {
    return async_request(masterpid, token, reply_hook, request_hook, move(more));
}

int pipeline::async_reply_worker(uint32_t destpid, uint32_t reply_token, const string& reply_hook, uint32_t success, const initializer_list< std::reference_wrapper< const string > >&& more) {
    if(reply_hook == "none") {
        return 0; // reply is not required
    }
    send_buffer.reset();
    binary_coder coder(send_buffer);
    coder <<= binary_coder::canary_begin;
	// send response
	// 0 = src pid, 1 = reply_hook, 2 = reply_token, 3 = success
    coder <<= getpid();      // send source pid
    coder <<= reply_hook;    // destination hook
    coder <<= reply_token;   // id/token
	coder <<= success;       // means success
	for(auto& x : more) {
        coder <<= x;
	}
    coder <<= binary_coder::canary_end;
    uint8_t* content = send_buffer.data();
    auto content_len = send_buffer.out_avail();
	DB_LOG(LOG_NOTICE, "[token%d]-response----------[dest:%d]-[bytes:%d][app:%s]{%X,%X}", reply_token, destpid, content_len, reply_token.c_str(), content[0], content[1]);
    pp_send(destpid, content, content_len);
	return 0;
}

