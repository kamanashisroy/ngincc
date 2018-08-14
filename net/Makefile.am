
TARGET_BIN=ngincc_net_main
TARGET_LAR=libngincc_net.la
bin_PROGRAMS=$(TARGET_BIN)
lib_LTLIBRARIES=$(TARGET_LAR)

NGINCC_NET_SOURCE_FILES = \
src/server_stack.cc \
src/raw_pipeline.cc \
src/tcp_listener_server.cc \
src/net_subsystem.cc

NGINCC_BASE_LDFLAGS=$(top_builddir)/base/libngincc_base.la
NGINCC_NET_INCLUDES=-Iinc -I$(top_builddir)/base/inc

NGINCC_CXXFLAGS=-std=c++17 -DGIT_COMMIT_VERSION=\"$(GIT_COMMIT_VERSION)\" -Wall
NGINCC_CXXFLAGS+=-ggdb3

libngincc_net_la_SOURCES = $(NGINCC_NET_SOURCE_FILES)
libngincc_net_la_CXXFLAGS = $(NGINCC_CXXFLAGS) $(NGINCC_NET_INCLUDES) 
libngincc_net_la_LDFLAGS = $(NGINCC_BASE_LDFLAGS)

ngincc_net_main_SOURCES = $(NGINCC_NET_SOURCE_FILES) src/main.cc
ngincc_net_main_CXXFLAGS = $(NGINCC_CXXFLAGS) -ggdb3 -DAROOP_BASIC -DAROOP_OPP_PROFILE $(NGINCC_NET_INCLUDES) 
ngincc_net_main_LDFLAGS= -static $(NGINCC_BASE_LDFLAGS)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libngincc_net@PACKAGE_SUFFIX@.pc

libngincc_net@PACKAGE_SUFFIX@.pc: libngincc_net.pc
	cp $< $@

#MAINTAINERCLEANFILES = ngincc.pc
