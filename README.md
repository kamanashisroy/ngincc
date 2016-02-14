
NginZ
==========

NginZ is a scalable application server. It is useful for instant messaging and audio/video communication.

Note
====

Plase update the port in inc/nginz\_config.h

Requirements
============

NginZ depends on the following packages,

- automake
- libtool
- pkg-config
- [Aroop Core](https://github.com/kamanashisroy/aroop_core)
- libmemcached

It it tested in Linux platform.

Building
========

The compilation command is. `./autogen.sh;make;make install;`

And the output binary is `nginz_main`, `nginz_debug_main`, `nginz_profiler_main`

Features
========

NginZ is equiped to serve as communication applications. It has,

- [Plugin](src/plugin.c) and dependency injection.
- Parallel processing support based on [token-ring and pipeline pattern](src/parallel/pipeline.c).
- It has scalability features. The requests are load-balanced in the worker processes.
- It has [memory profiler](src/net/chat/profiler.c).
- It has [event-loop](src/event_loop.c) module to handle user data in [fibers](src/fiber.c).
- It has [command shell](src/shake.c) to diagnose the server.
- Writing new feature for chat server needs very less code(see the following ..). 

Dependency injection
====================

As a framework it supports dependency injection by plugin-spaces/extension-points. It makes less coupling and strong cohesion possible. And the result is less code and easily maintainable source for each features.

| Module/Source | Lines of code |
| --- | --- |
| ./src/net/chat/room.c | 182 |
| ./src/net/chat/quit.c | 43 |
| ./src/net/chat/leave.c | 34 |
| ./src/net/chat/broadcast.c | 182 |
| ./src/net/chat/join.c | 116 |
| ./src/net/chat/uptime.c | 48 |
| ./src/net/chat/profiler.c | 51 |
| ./src/net/chat/chat\_plugin\_manager.c | 118 |
| ./src/net/chat/welcome.c | 86 |
| ./src/net/chat/hiddenjoin.c | 75 |
| ./src/net/chat/user.c | 56 |


livedemo
========

```
telnet ec2-54-191-149-216.us-west-2.compute.amazonaws.com 9399
Trying 54.191.149.216...
Connected to ec2-54-191-149-216.us-west-2.compute.amazonaws.com.
Escape character is '^]'.
Welcome to NginZ chat server
Login name?
foo
Welcome foo!
/help
...
/rooms
Active rooms are:
	* ONE (1)
	* TWO
	* THREE
	* FOUR
	* FIVE
end of list.
/join ONE
Trying ...(14364)
Entering room:ONE
	* Shuva
	* foo(** this is you)
end of list
Shuva:hi foo
Shuva:I have to leave
Shuva:Have a nice day
	* user has left chat:Shuva
/uptime
...
/profiler
...
/quit
	* user has left chat:foo
BYE
Connection closed by foreign host.
```

Command shell
=============

```
socat - UNIX-CLIENT:/tmp/nginz.sock
help
plugin	shake	shake/plugin	src/plugin_manager.c
It dumps the avilable plugins
....
```

