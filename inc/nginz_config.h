#ifndef NGINZ_CONFIG_H
#define NGINZ_CONFIG_H

C_CAPSULE_START

#define NGINZ_INLINE inline

/**
 * Parallel computing
 */
// XXX token ring parallel processing requires that there are at least 2 processes
enum parallel_config {
	NGINZ_NUMBER_OF_PROCESSORS = 5,
};

enum binary_coder_config {
	NGINZ_MAX_BINARY_MSG_LEN = 1024,
};

#include <poll.h> // to define POLLIN, POLLPRI, POLLHUP
/**
 * Event loop configuration
 */
enum event_loop_config {
	MAX_POLL_FD = 20000, // for some implementation this value must by devisible by some POLL_PARTITION value
	NGINZ_POLL_LISTEN_FLAGS = POLLIN | POLLPRI | POLLHUP,
	NGINZ_POLL_ALL_FLAGS = POLLIN | POLLPRI | POLLHUP,
};

/**
 * Protocol implemenation
 */
enum protocol_config {
	NGINZ_MAX_PROTO = 4,
	NGINZ_CHAT_PORT = 9399,
	NGINZ_HTTP_PORT = 80,
};

/**
 * TCP listen config
 */
enum tcp_config {
	NGINZ_TCP_LISTENER_BACKLOG = 1024<<1, /* XXX we are setting this too high */
};

/**
 * object pool factory
 */
#define NGINZ_FACTORY_CREATE(obuff, psize, objsize, callback) ({OPP_PFACTORY_CREATE_FULL(obuff, psize, objsize, 1, OPPF_SWEEP_ON_UNREF, callback);})
#define NGINZ_EXTENDED_FACTORY_CREATE(obuff, psize, objsize, callback) ({OPP_PFACTORY_CREATE_FULL(obuff, psize, objsize, 1, OPPF_EXTENDED | OPPF_SWEEP_ON_UNREF, callback);})
#define NGINZ_HASHABLE_FACTORY_CREATE(obuff, psize, objsize, callback) ({OPP_PFACTORY_CREATE_FULL(obuff, psize, objsize, 1, OPPF_EXTENDED | OPPF_SWEEP_ON_UNREF, callback);})
#define NGINZ_SEARCHABLE_FACTORY_CREATE(obuff, psize, objsize, callback) ({OPP_PFACTORY_CREATE_FULL(obuff, psize, objsize, 1, OPPF_SEARCHABLE | OPPF_EXTENDED | OPPF_SWEEP_ON_UNREF, callback);})

/**
 * Chat server configuration
 */
enum chat_config {
	NGINZ_MAX_CHAT_USER_NAME_SIZE = 32,
	NGINZ_MAX_CHAT_ROOM_NAME_SIZE = 32,
	NGINZ_MAX_CHAT_MSG_SIZE = 255,
	NGINZ_MAX_WEBCHAT_MSG_SIZE = 1024,
	NGINZ_MAX_WEBCHAT_SID_SIZE = 32,
	NGINZ_MAX_CHAT_ROOM_USER = 32,
	//NGINZ_MAX_COMMANDS = 16,
};

/**
 * Http server configuration
 */
enum http_config {
	NGINZ_MAX_HTTP_MSG_SIZE = 1024,
	NGINZ_MAX_HTTP_HEADER_SIZE = 1024,
};

//#define NGINZ_EVENT_DEBUG 

#ifdef AROOP_MODULE_NAME
#undef AROOP_MODULE_NAME
#endif
#define AROOP_MODULE_NAME "NginZ"

/**************************************************************************/
/***************************** enable modules *****************************/
/**************************************************************************/
// http module allows to load http
#define HAS_HTTP_MODULE

// chat module allows telnet chat
#define HAS_CHAT_MODULE
// web chat module tunnels chat over htp
#define HAS_WEB_CHAT_MODULE
//#define HAS_MEMCACHED_MODULE


// fixup
#ifndef HAS_CHAT_MODULE
#undef HAS_WEB_CHAT_MODULE
#endif
#ifndef HAS_HTTP_MODULE
#undef HAS_WEB_CHAT_MODULE
#endif
/**************************************************************************/


C_CAPSULE_END

#endif // NGINZ_CONFIG_H
