
#include <memory>
#include <vector>
#include <string>

#include "module.hxx"
//#include "server_stack.hxx"
//#include "binary_coder.hxx"
#include "chat/chat_connection.hxx"
#include "chat/chat_user.hxx"

using std::string;
using std::ostringstream;
using std::endl;
using std::vector;
using ngincc::core::plugin_manager;
using namespace ngincc::apps::chat;
using namespace std::placeholders;

static int build_name_key(aroop_txt_t*name, aroop_txt_t*output) {
	aroop_txt_concat_string(output, USER_PREFIX);
	aroop_txt_concat(output, name);
	aroop_txt_zero_terminate(output);
	return 0;
}


