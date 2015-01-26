#                            Package   : omniEvents
# src/Makefile               Created   : 2003/10/31
#                            Author    : Alex Tingle
#
#    Copyright (C) 2003 Alex Tingle.
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

LIBRARY_SOURCE_FILES = \
 ConsumerAdmin.cc \
 EventChannel.cc \
 EventChannelFactory.cc \
 EventQueue.cc \
 Filter.cc \
 Orb.cc \
 PersistNode.cc \
 ProxyManager.cc \
 ProxyPullConsumer.cc \
 ProxyPullSupplier.cc \
 ProxyPushConsumer.cc \
 ProxyPushSupplier.cc \
 Servant.cc \
 SupplierAdmin.cc \
 omniEventsLog.cc \
 version.cc \

DAEMON_SOURCE_FILES := \
 $(LIBRARY_SOURCE_FILES) \
 getopt.cc \
 main.cc \
 naming.cc \
 omniEvents.cc \

LIBRARY_HEADER_FILES := $(patsubst %.cc,%.h,$(LIBRARY_SOURCE_FILES)) \
 Callback.h \
 Mapper.h \
 config.h \
 defaults.h \
 scour.h \

include ../config.mk

ifneq "$(strip $(HAVE_GETHOSTNAME))" "yes"
  LIBRARY_HEADER_FILES += gethostname.h
endif

INSTALLED_HEADER_FILES := \
 $(patsubst %,$(INSTALL_INCLUDE)/$(OMNIEVENTS)/%,$(LIBRARY_HEADER_FILES))

ifneq ($(BUILD_FOR_WINDOWS),)
  DAEMON_SOURCE_FILES += daemon_windows.cc
else
  DAEMON_SOURCE_FILES += daemon_unix.cc
endif

DAEMON_EXE     := $(OMNIEVENTS)$(EXEEXT)
LIBRARY        := $(call SharedLibName,$(OMNIEVENTS))
LIBRARY_SONAME := $(call SharedLibSoName,$(OMNIEVENTS),$(PACKAGE_VERSION))

ARCHIVES   = ../idl/$(CLIENT_LIB) \

all: exe lib

install: all install_dirs $(INSTALLED_HEADER_FILES)
	$(INSTALL) $(DAEMON_EXE) $(INSTALL_SBIN)
	$(call InstallSharedLib,$(OMNIEVENTS),$(PACKAGE_VERSION),$(INSTALL_LIB))

$(INSTALLED_HEADER_FILES): $(INSTALL_INCLUDE)/$(OMNIEVENTS)/%: % install_dirs
	$(INSTALL) -m 644 $< $(INSTALL_INCLUDE)/$(OMNIEVENTS)

install_dirs:
	$(INSTALL) -d $(INSTALL_SBIN)
	$(INSTALL) -d $(INSTALL_LIB)
	$(INSTALL) -d $(INSTALL_INCLUDE)/$(OMNIEVENTS)

uninstall:
	$(RMDIR) $(INSTALL_INCLUDE)/$(OMNIEVENTS)
	$(RM) $(INSTALL_SBIN)/$(DAEMON_EXE)
	# Don't uninstall shared library.

exe: $(DAEMON_EXE)
$(DAEMON_EXE): $(patsubst %.cc,%.$(OBJEXT),$(DAEMON_SOURCE_FILES))
	$(call CxxBuildExecutable,$@,$^ $(ARCHIVES))

lib: $(LIBRARY)
$(LIBRARY): $(patsubst %.cc,%.$(OBJEXT),$(LIBRARY_SOURCE_FILES))
	$(call CxxBuildShared,$@,$^ $(ARCHIVES),,$(LIBRARY_SONAME))

clean:
	$(RM) $(DAEMON_EXE) $(LITE_EXE) $(LIBRARY) *.$(OBJEXT)

##
## A slim version of the daemon that uses the shared library. (Only on Unix).
LITE_EXE          := oelite$(EXEEXT)
LITE_SOURCE_FILES := getopt.cc omniEvents.cc naming.cc main.cc daemon_unix.cc
LITE_OBJECTS      := $(patsubst %.cc,%.$(OBJEXT),$(LITE_SOURCE_FILES))
$(LITE_EXE): $(LITE_OBJECTS) lib
	$(call CxxBuildExecutable,$@,$(LITE_OBJECTS) -L. -l$(OMNIEVENTS))

include deps.mk

.PHONY: all install install_dirs uninstall exe lib clean
