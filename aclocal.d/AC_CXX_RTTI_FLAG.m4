dnl 
dnl AC_CXX_PIC_FLAG
dnl
dnl Description
dnl
dnl  Autodetects C++ `Rel Time Type Information' (RTTI) flag.
dnl  Adds whatever CXXFLAGS are needed to ensure than C++ object code can
dnl  use dynamic_cast<> and typeid.
dnl 
dnl Copyright (C) 2005, Alex Tingle <alex.autoconf@firetree.net>
dnl 
dnl License:
dnl GNU General Public License
dnl [http://www.gnu.org/software/ac-archive/htmldoc/COPYING.html]
dnl with this special exception
dnl [http://www.gnu.org/software/ac-archive/htmldoc/COPYING-Exception.html]. 
dnl 

AC_DEFUN([AC_CXX_RTTI_FLAG],[
  AC_REQUIRE([AC_CANONICAL_HOST])
  AC_REQUIRE([AC_CXX_IDENTITY])
  AC_CACHE_CHECK([for C++ RTTI flag],
    ac_cv_cxx_rtti_flag,
    [
      ac_cxx_rtti_save_cxxflags="$CXXFLAGS"
      ac_cv_cxx_rtti_flag="none needed"

      case "$ac_cv_cxx_identity" in
        GNU-g++-*-*-*) ac_cv_cxx_rtti_flag="on by default" ;; # -fno-rtti
         HP-aCC-*-*-*) ac_cv_cxx_rtti_flag="none needed" ;;
           *-CC-*-*-*) ac_cv_cxx_rtti_flag="on by default" ;;
        DEC-cxx-*-*-*) ac_cv_cxx_rtti_flag="on by default" ;; # -[no]rtti
        IBM-xlC-*-*-*) ac_cv_cxx_rtti_flag="-qrtti=all" ;;
      esac

      # Finish up by adding the PIC flag to CXXFLAGS
      if test "$ac_cv_cxx_rtti_flag" = "none needed" \
           -o "$ac_cv_cxx_rtti_flag" = "on by default"
      then
        CXXFLAGS="$ac_cxx_rtti_save_cxxflags"
      else
        CXXFLAGS="$ac_cxx_rtti_save_cxxflags $ac_cv_cxx_rtti_flag"
      fi
    ])
])
