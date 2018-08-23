
TARGET_BIN=ngincc_http_main
TARGET_LAR=libngincc_http.la
bin_PROGRAMS=$(TARGET_BIN)
lib_LTLIBRARIES=$(TARGET_LAR)

NGINCC_HTTP_SOURCE_FILES = \
src/http_subsystem.cc	src/http_server_stack.cc	src/http_connection.cc

NGINCC_HTTP_LDFLAGS=$(top_builddir)/base/libngincc_base.la $(top_builddir)/net/libngincc_net.la $(top_builddir)/apps/load_balancer/libngincc_lb.la
NGINCC_HTTP_INCLUDES=-Iinc -I$(top_builddir)/base/inc -I$(top_builddir)/net/inc  -I$(top_builddir)/apps/load_balancer/inc 


NGINCC_CXXFLAGS=-std=c++17 -DGIT_COMMIT_VERSION=\"$(GIT_COMMIT_VERSION)\" -Wall
NGINCC_CXXFLAGS+=-ggdb3

libngincc_http_la_SOURCES = $(NGINCC_HTTP_SOURCE_FILES)
libngincc_http_la_CXXFLAGS = $(NGINCC_CXXFLAGS) $(NGINCC_HTTP_INCLUDES)
libngincc_http_la_LDFLAGS = $(NGINCC_HTTP_LDFLAGS)

ngincc_http_main_LDADD = ${NGINCC_HTTP_LDFLAGS}
ngincc_http_main_SOURCES = $(NGINCC_HTTP_SOURCE_FILES) src/main.cc
ngincc_http_main_CXXFLAGS = $(NGINCC_CXXFLAGS) $(NGINCC_HTTP_INCLUDES) 
ngincc_http_main_LDFLAGS= -static $(NGINCC_HTTP_LDFLAGS)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libngincc_http@PACKAGE_SUFFIX@.pc

libngincc_http@PACKAGE_SUFFIX@.pc: libngincc_http.pc
	cp $< $@

#MAINTAINERCLEANFILES = ngincc.pc
