
SUBDIRS=tests

#TARGET_BIN=ngincc_db_main
TARGET_LAR=libngincc_db.la
#bin_PROGRAMS=$(TARGET_BIN)
lib_LTLIBRARIES=$(TARGET_LAR)

NGINCC_DB_SOURCE_FILES = src/async_db.cc src/async_db_master.cc

NGINCC_BASE_LDFLAGS=$(top_builddir)/base/libngincc_base.la
NGINCC_DB_INCLUDES=-Iinc -I$(top_builddir)/base/inc

NGINCC_CXXFLAGS=-std=c++17 -DGIT_COMMIT_VERSION=\"$(GIT_COMMIT_VERSION)\" -Wall
NGINCC_CXXFLAGS+=-ggdb3

libngincc_db_la_SOURCES = $(NGINCC_DB_SOURCE_FILES)
libngincc_db_la_CPPFLAGS = $(NGINCC_CXXFLAGS) $(NGINCC_DB_INCLUDES) 
libngincc_db_la_LDFLAGS = $(NGINCC_BASE_LDFLAGS)

#ngincc_db_main_SOURCES = $(NGINCC_DB_SOURCE_FILES) src/main.c
#ngincc_db_main_CPPFLAGS = $(NGINCC_CXXFLAGS) -ggdb3 -DAROOP_BASIC -DAROOP_OPP_PROFILE $(NGINCC_DB_INCLUDES) 
#ngincc_db_main_LDFLAGS= -static $(MEMCACHED_LIBS) $(NGINCC_BASE_LDFLAGS) @AROOP_DEBUG_LIBS@ 

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libngincc_db@PACKAGE_SUFFIX@.pc

libngincc_db@PACKAGE_SUFFIX@.pc: libngincc_db.pc
	cp $< $@

#MAINTAINERCLEANFILES = ngincc.pc
