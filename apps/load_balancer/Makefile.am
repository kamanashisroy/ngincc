
TARGET_LAR=libngincc_lb.la
bin_PROGRAMS=$(TARGET_BIN)
lib_LTLIBRARIES=$(TARGET_LAR)

NGINCC_LOAD_BALANCER_SOURCE_FILES = \
src/load_balancer.cc

NGINCC_BASE_LDFLAGS=$(top_builddir)/base/libngincc_base.la
NGINCC_NET_LDFLAGS=$(top_builddir)/net/libngincc_net.la
NGINCC_LOAD_BALANCER_INCLUDES=-Iinc -I$(top_builddir)/base/inc -I$(top_builddir)/net/inc

NGINCC_CXXFLAGS=-std=c++17 -DGIT_COMMIT_VERSION=\"$(GIT_COMMIT_VERSION)\" -Wall
NGINCC_CXXFLAGS+=-ggdb3

libngincc_lb_la_SOURCES = $(NGINCC_LOAD_BALANCER_SOURCE_FILES)
libngincc_lb_la_CXXFLAGS = $(NGINCC_CXXFLAGS) $(NGINCC_LOAD_BALANCER_INCLUDES)
libngincc_lb_la_LDFLAGS = $(NGINCC_BASE_LDFLAGS) $(NGINCC_NET_LDFLAGS)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libngincc_lb@PACKAGE_SUFFIX@.pc

libngincc_lb@PACKAGE_SUFFIX@.pc: libngincc_lb.pc
	cp $< $@

#MAINTAINERCLEANFILES = ngincc.pc
