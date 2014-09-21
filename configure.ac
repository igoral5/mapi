dnl Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(mapi, 0.01, igoral@gmail.com)
AC_CONFIG_SRCDIR([src/main.cpp])
AC_CONFIG_AUX_DIR([build-aux])
AC_CANONICAL_SYSTEM
AM_INIT_AUTOMAKE([-Wall -Werror])
AM_GNU_GETTEXT_VERSION([0.18])
AM_GNU_GETTEXT([external])
AC_PROG_CXX
AC_LANG([C++])
AC_CHECK_HEADERS([boost/thread/mutex.hpp],,AC_MSG_ERROR(Not found header files boost/thread/mutex.hpp))
AC_CHECK_HEADERS([boost/thread/thread.hpp],,AC_MSG_ERROR(Not found header files boost/thread/thread.hpp))
LIBS_SAVED="$LIBS"
LIBS="$LIBS_SAVED -lboost_thread -lboost_system"
AC_MSG_CHECKING([for libboost_thread library])
AC_TRY_LINK([#include <boost/thread/thread.hpp>],
	[ boost::thread current_thread ],
	[ AC_MSG_RESULT(yes)
	  BOOST_THREAD_LIB="-lboost_thread -lboost_system"
	],
	[ AC_MSG_RESULT(no)
	  AC_MSG_ERROR(Unable to link with the libboost_thread library)
	])
LIBS="$LIBS_SAVED"
AC_SUBST(BOOST_THREAD_LIB)
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES(Makefile src/Makefile po/Makefile.in)
AC_OUTPUT
