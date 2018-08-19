# please refer to https://wiki.gnome.org/Projects/Vala/GameDevelopmentSeries/Setup

m4_define([ngincc_major], [0])
m4_define([ngincc_minor], [0])
m4_define([ngincc_micro], [1])

m4_define([ngincc_version], [ngincc_major.ngincc_minor.ngincc_micro])
m4_define([ngincc_suffix], [-ngincc_version])
m4_define([ngincc_api_version], [0.0])

m4_define([ngincc_name], [ngincc])

AC_PREREQ([2.64])
AC_INIT([NginCC], [ngincc_version], [kamanashisroy@github.com])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([dist-bzip2 subdir-objects])
AM_MAINTAINER_MODE
#AM_PATH_CHECK()
PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])

NGINZ_MAJOR_VERSION=ngincc_major
NGINZ_MINOR_VERSION=ngincc_minor
NGINZ_MICRO_VERSION=ngincc_micro
NGINZ_VERSION=ngincc_version
NGINZ_API_VERSION=ngincc_api_version
PACKAGE_SUFFIX=ngincc_suffix
AC_SUBST(PACKAGE_SUFFIX)
AC_SUBST(NGINZ_MAJOR_VERSION)
AC_SUBST(NGINZ_MINOR_VERSION)
AC_SUBST(NGINZ_MICRO_VERSION)
AC_SUBST(NGINZ_VERSION)
AC_SUBST(NGINZ_API_VERSION)

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])


# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])

# It is also possible to add this flag by seting the variables
AC_ARG_ENABLE([production],
    AS_HELP_STRING([--enable-production], [CXXFLAGS='-O2']))

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_RANLIB

AC_CONFIG_FILES([
	base/Makefile
	base/libngincc_base.pc
	net/Makefile
	net/libngincc_net.pc
	db/Makefile
	db/libngincc_db.pc
	apps/Makefile
	apps/load_balancer/Makefile
	apps/load_balancer/libngincc_lb.pc
	apps/http/Makefile
	apps/http/libngincc_http.pc
	apps/chat/Makefile
	Makefile
])

AC_OUTPUT
