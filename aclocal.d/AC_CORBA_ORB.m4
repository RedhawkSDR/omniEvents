dnl
dnl  AC_CORBA_ORB
dnl
dnl Description
dnl 
dnl  Tests for a linkable CORBA ORB. Currentlly only finds omniORB3 or
dnl  omniORB4. Sets the output variable `CORBA_ORB', sets variables CPPFLAGS,
dnl  LIBS & LDFLAGS. Sets pthread & socket options if necessary.
dnl
dnl Copyright (C) 2003, Alex Tingle <alex.autoconf@firetree.net>
dnl 
dnl License:
dnl GNU General Public License
dnl [http://www.gnu.org/software/ac-archive/htmldoc/COPYING.html]
dnl with this special exception
dnl [http://www.gnu.org/software/ac-archive/htmldoc/COPYING-Exception.html]. 
dnl 

AC_DEFUN([AC_CORBA_ORB],[

  AC_ARG_WITH([omniorb],
    [[  --with-omniorb=PATH     set the path to the local omniORB installation
                          [$OMNIORBBASE].]],
    [OMNIORBBASE=$withval]
  )

  if test "x$CORBA_ORB" = x; then
    AC_CORBA_ORB_OMNIORB4
  fi
  if test "x$CORBA_ORB" = x; then
    AC_CORBA_ORB_OMNIORB3
  fi
])


dnl
dnl AC_CORBA_SOCKET_NSL
dnl Small wrapper around ETR_SOCKET_NSL. Automatically adds the result to LIBS.
dnl 

AC_DEFUN([AC_CORBA_SOCKET_NSL],[
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([ETR_SOCKET_NSL])
  if test "x$ETR_SOCKET_LIBS" != x; then
    LIBS="$LIBS $ETR_SOCKET_LIBS"
  fi
])
