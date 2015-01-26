#                            Package   : omniEvents
# config.mk.in               Created   : 2003/10/31
#                            Author    : Alex Tingle
#
#    Copyright (C) 2003, 2004 Alex Tingle.
#
#    This file is part of the omniEvents application.
#
#    omniEvents is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Lesser General Public
#    License as published by the Free Software Foundation; either
#    version 2.1 of the License, or (at your option) any later version.
#
#    omniEvents is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Lesser General Public License for more details.
#
#    You should have received a copy of the GNU Lesser General Public
#    License along with this library; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

##
## MANUALLY SET THE BASE DIRECTORY FOR OMNIORB & OMNIEVENTS:
## 
OMNIORB_BASE    = C:\Progra~1\omniORB-4.0.6
OMNIORB_LIBS    = omniORB406_rt.lib omniDynamic406_rt.lib omnithread31_rt.lib
OMNIEVENTS_BASE = C:\omniEvents-2_6_2
##

CORBA_ORB = omniORB4
BUILD_FOR_WINDOWS := yes

# Punctuation
EMPTY :=
SPACE := $(EMPTY) $(EMPTY)

# Platform dependent file extensions.
OBJEXT            := obj
EXEEXT            := .exe
SOEXT             := .dll
StaticLib          = $(1).lib

# Name of the application and libraries that we will build.
OMNIEVENTS        := omniEvents
CLIENT_LIB        := $(call StaticLib,$(OMNIEVENTS)Cl)
PACKAGE_VERSION   := 2.6.2
HAVE_GETHOSTNAME  := no # Need to install a substitute.

# Define installation directories.
prefix            := C:\events
exec_prefix       := ${prefix}

INSTALL_BIN       := $(DESTDIR)${exec_prefix}/bin
INSTALL_SBIN      := $(DESTDIR)${exec_prefix}/sbin
INSTALL_LIB       := $(DESTDIR)${exec_prefix}/lib
INSTALL_INCLUDE   := $(DESTDIR)${prefix}/include
INSTALL_IDL       := $(DESTDIR)${prefix}/share/idl
INSTALL_ETC       := $(DESTDIR)/etc
SYSCONFIG_DIR     := $(DESTDIR)/etc/default

# Set up install & clean programs.
INSTALL = rem # No install on windows yet
RM      = del /F
RMDIR   = del /F

##
## IDL compiler.
##

IDL         := omniidl
IDL_COS_DIR := '$(OMNIORB_BASE)\idl'
IDLFLAGS     = -bcxx  # <--- backend selection should be omniidl's first option.
IDLFLAGS    += -Wba -Wbh='.hh' -Wbs='.cc' -Wbd='DynSK.cc' -WbBOA -Wbuse_quotes
IDLFLAGS    += -I$(IDL_COS_DIR)\COS

# The IDL compiler doesn't write code that uses config.h, so we have to compile
# the generated files with a couple of extra options:
EXTRA_IDL_CXXFLAGS = -D__WIN32__ -D__x86__=1 -D__OSVERSION__=5 \
                     -D_WIN32_WINNT=0x0400 -D__NT__ -D_COS_LIBRARY

##
## C++ compiler:  $(CXX) $(CPPFLAGS) $(CXXFLAGS) source
##

CXX       = cl -nologo
CXXFLAGS += -GX -MD -GR # CXXFLAGS

CPPFLAGS += -TP -I$(subst \,\\,$(OMNIORB_BASE))\\include # CPPFLAGS
CPPFLAGS += $(EXTRA_IDL_CXXFLAGS) -DHAVE_CONFIG_H # DEFS
CPPFLAGS += -I../idl -I../src
CPPFLAGS += -DENABLE_CLIENT_IR_SUPPORT=1 -DOMNIEVENTS_REAL_TIME_PUSH=1

##
## C++ Linker:  $(LD) objects $(LDFLAGS) $(LDOUT)target $(LDLIBS)
##

LD        = $(CXX)
LDFLAGS  += -GX -MD -GR # CXXFLAGS
LDFLAGS  += -link # LDFLAGS
LDLIBS   += /LIBPATH:$(subst \,\\,$(OMNIORB_BASE))\\lib\\x86_win32 \
  ws2_32.lib mswsock.lib advapi32.lib $(OMNIORB_LIBS) # LIBS

##
## Command to make a symbolic link from $(1) to $(2)
##

MakeSymLink = # No symbolic links on Windows.

##
## Command to build C++ static archive (library)
## $(1) - archive/library (output) filename
## $(2) - objects and static libraries to include
##

CxxBuildStatic = link /lib /nologo /OUT:$(1) $(2)

##
## Command to build C++ executable.
## $(1) - executable (output) filename
## $(2) - objects and static libraries to include
##

CxxBuildExecutable = $(LD) $(2) $(LDFLAGS) /OUT:$(1) $(LDLIBS)

##
## Command to build C++ shared libraries.
## $(1) - output filename (.so)
## $(2) - objects
## $(3) - static libraries to include
## $(4) - soname
##

CxxBuildShared = # No shared libraries on windows yet.

##
## Command to install C++ shared libraries.
## $(1) - root library name
## $(2) - package version (MAJOR.MINOR.MICRO)
## $(3) - target directory
##

SharedLibName     = $(1)$(SOEXT)
SharedLibFullName = $(1).$(2)$(SOEXT)
SharedLibSoName   = $(SharedLibFullName)

InstallSharedLib = # No install OR shared libs on windows yet!!

##
## Some compilers & linkers need to be told where to store
## template instantiations.
##

CXXFLAGS_PTR :=
CXX_REPOSITORY := $(OMNIEVENTS_BASE)/templates.d
ifneq ("$(CXXFLAGS_PTR)","")
  CXXFLAGS += $(CXX_REPOSITORY)
  LDFLAGS  += $(CXX_REPOSITORY)
endif

##
## Suffix rules for compiling .idl & .cc files.
##

.SUFFIXES: .idl .cc .$(OBJEXT)

.idl.cc:
	$(IDL) $(IDLFLAGS) $<

.cc.$(OBJEXT):
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<

# (end)
