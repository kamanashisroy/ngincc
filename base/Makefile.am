
SUBDIRS= tests

TARGET_BIN=ngincc_base_main
TARGET_LAR=libngincc_base.la
bin_PROGRAMS=$(TARGET_BIN)
lib_LTLIBRARIES=$(TARGET_LAR)

NGINCC_SOURCE_FILES = \
src/worker.cc    src/plugin_manager.cc \
src/lazy_worker.cc \
src/binary_coder.cc \
src/shake/shake_intrprtr.cc \
src/shake/help.cc \
src/shake/test.cc \
src/shake/quitall.cc \
src/shake/enumerate.cc \
src/parallel/pipeline.cc \
src/parallel/fork.cc \
src/parallel/ping.cc \
src/parallel/async_request.cc \
src/base_subsystem.cc

EVENT_LOOP_POLL = src/event_loop_poll.cc
EVENT_LOOP_PARTITIONED_OPENMP_POLL = src/event_loop_partitioned_openmp_poll.cc

pkgincludedir=$(includedir)/ngincc_base@PACKAGE_SUFFIX@
nobase_pkginclude_HEADERS= \
inc/event_loop.hxx \
inc/worker.hxx      inc/plugin_manager.hxx      inc/lazy_worker.hxx

GIT_COMMIT_VERSION=`git describe --dirty --always --tags`

NGINCC_INCLUDES=-Iinc
NGINCC_CXXFLAGS=-std=c++17 -DGIT_COMMIT_VERSION=\"$(GIT_COMMIT_VERSION)\" -Wall
NGINCC_CXXFLAGS+=-ggdb3

libngincc_base_la_SOURCES = $(NGINCC_SOURCE_FILES) $(EVENT_LOOP_POLL)
libngincc_base_la_CXXFLAGS = $(NGINCC_CXXFLAGS) $(NGINCC_INCLUDES)

# binary executables
ngincc_base_main_SOURCES= $(NGINCC_SOURCE_FILES) src/main.cc $(EVENT_LOOP_POLL)
ngincc_base_main_CXXFLAGS= $(NGINCC_CXXFLAGS) $(NGINCC_INCLUDES)
ngincc_base_main_LDFLAGS=

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libngincc_base@PACKAGE_SUFFIX@.pc

libngincc_base@PACKAGE_SUFFIX@.pc: libngincc_base.pc
	cp $< $@

#MAINTAINERCLEANFILES = ngincc.pc
