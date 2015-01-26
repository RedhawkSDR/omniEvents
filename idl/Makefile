#                            Package   : omniEvents
# idl/Makefile               Created   : 2003/10/31
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

IDL_FILES := \
 FT.idl \
 omniEvents.idl \
 EventChannelAdmin.idl \

IDL_FILES_COS := \
 CosLifeCycle.idl \
 CosEventComm.idl \
 CosEventChannelAdmin.idl \
 CosTypedEventComm.idl \
 CosTypedEventChannelAdmin.idl \

include ../config.mk

HH_FILES     := $(patsubst %.idl,%.hh,$(IDL_FILES))
HH_FILES_COS := $(patsubst %.idl,%.hh,$(IDL_FILES_COS))

CC_FILES     := $(patsubst %.hh,%.cc,$(HH_FILES))
CC_FILES_COS := $(patsubst %.hh,%.cc,$(HH_FILES_COS))

SOURCE_FILES := \
 $(patsubst %.hh,%.cc,     $(HH_FILES) $(HH_FILES_COS)) \
 $(patsubst %.hh,%DynSK.cc,$(HH_FILES) $(HH_FILES_COS))
OBJECT_FILES := $(patsubst %.cc,%.$(OBJEXT),$(SOURCE_FILES))

INSTALLED_IDL_FILES := \
  $(patsubst %,$(INSTALL_IDL)/$(OMNIEVENTS)/%,$(IDL_FILES))
INSTALLED_HH_FILES := \
  $(patsubst %,$(INSTALL_INCLUDE)/$(OMNIEVENTS)/%,\
    $(HH_FILES) $(HH_FILES_COS) COS_sysdep.h)

# The IDL compiler doesn't use config.h, so we have to give it some
# defines on the command line.
CXXFLAGS += -I. $(EXTRA_IDL_CXXFLAGS)

all: compile_idl_files $(CLIENT_LIB)

install: all install_dirs $(INSTALLED_HH_FILES) $(INSTALLED_IDL_FILES)
	$(INSTALL) $(CLIENT_LIB) $(INSTALL_LIB)

# NOte: Install .hh files in $(INSTALL_INCLUDE) - for backward compatibility:
$(INSTALLED_HH_FILES): $(INSTALL_INCLUDE)/$(OMNIEVENTS)/%: % install_dirs
	$(INSTALL) -m 644 $< $(INSTALL_INCLUDE)/$(OMNIEVENTS)
	$(INSTALL) -m 644 $< $(INSTALL_INCLUDE) # Deprecated header

$(INSTALLED_IDL_FILES): $(INSTALL_IDL)/$(OMNIEVENTS)/%: % install_dirs
	$(INSTALL) -m 644 $< $(INSTALL_IDL)/$(OMNIEVENTS)

install_dirs:
	$(INSTALL) -d $(INSTALL_LIB)
	$(INSTALL) -d $(INSTALL_INCLUDE)/$(OMNIEVENTS)
	$(INSTALL) -d $(INSTALL_IDL)/$(OMNIEVENTS)

uninstall:
	$(RM) $(INSTALL_LIB)/$(CLIENT_LIB)
	$(RMDIR) $(INSTALL_INCLUDE)/$(OMNIEVENTS)
	$(RMDIR) $(INSTALL_IDL)/$(OMNIEVENTS)
	# Uninstall backward compatibility .hh files:
	$(RM) $(patsubst %,$(INSTALL_INCLUDE)/%,$(HH_FILES))
	$(RM) $(patsubst %,$(INSTALL_INCLUDE)/%,$(HH_FILES_COS) COS_sysdep.h)

$(CLIENT_LIB): $(OBJECT_FILES)
	$(call CxxBuildStatic,$@,$^)

$(CC_FILES_COS):
	$(IDL) $(IDLFLAGS) $(patsubst %.cc,$(IDL_COS_DIR)/COS/%.idl,$@)

compile_idl_files: $(CC_FILES_COS) $(CC_FILES)

clean:
	$(RM) *.cc *.hh $(OBJECT_FILES) $(CLIENT_LIB)

include deps.mk

.PHONY: all install install_dirs uninstall compile_idl_files clean
