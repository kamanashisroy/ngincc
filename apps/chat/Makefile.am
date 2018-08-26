
TARGET_BIN=ngincc_chat_main
TARGET_LAR=libngincc_chat.la
bin_PROGRAMS=$(TARGET_BIN)
lib_LTLIBRARIES=$(TARGET_LAR)

NGINCC_CHAT_SOURCE_FILES = \
src/chat_server_stack.cc \
src/chat_connection.cc \
src/chat_subsystem.cc \
src/chat_factory.cc \
src/connection_state_connected.cc \
src/connection_state_logged_in.cc \
src/connection_state_in_room.cc \
src/connection_state_quitting.cc \
src/join_room.cc \
src/quit.cc \
src/help.cc \
src/room_list.cc \
src/room_list_master.cc


#src/room.cc \
#src/quit.cc \
#src/join.cc \
#src/broadcast.cc \
#src/leave.cc \
#src/user.cc \
#src/hiddenjoin.cc \
#src/uptime.cc \
#src/profiler.cc \
#src/version.cc \
#src/private_message.cc \
#src/room_master.cc

NGINCC_CHAT_LDFLAGS=$(top_builddir)/base/libngincc_base.la $(top_builddir)/net/libngincc_net.la $(top_builddir)/apps/load_balancer/libngincc_lb.la $(top_builddir)/db/libngincc_db.la
NGINCC_CHAT_INCLUDES=-Iinc -I$(top_builddir)/base/inc -I$(top_builddir)/net/inc -I$(top_builddir)/apps/load_balancer/inc  -I$(top_builddir)/db/inc 

NGINCC_CXXFLAGS=-std=c++17 -DGIT_COMMIT_VERSION=\"$(GIT_COMMIT_VERSION)\" -Wall
NGINCC_CXXFLAGS+=-ggdb3


libngincc_chat_la_SOURCES = $(NGINCC_CHAT_SOURCE_FILES)
libngincc_chat_la_CXXFLAGS = $(NGINCC_CXXFLAGS) $(NGINCC_CHAT_INCLUDES)
libngincc_chat_la_LDFLAGS = $(NGINCC_CHAT_LDFLAGS)

ngincc_chat_main_LDADD = $(NGINCC_CHAT_LDFLAGS)
ngincc_chat_main_SOURCES = $(NGINCC_CHAT_SOURCE_FILES) src/main.cc
ngincc_chat_main_CXXFLAGS = $(NGINCC_CXXFLAGS) -O2 $(NGINCC_CHAT_INCLUDES) 
ngincc_chat_main_LDFLAGS= -static $(NGINCC_CHAT_LDFLAGS)

#MAINTAINERCLEANFILES = ngincc.pc
