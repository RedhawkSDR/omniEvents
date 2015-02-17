#                            Package   : omniEvents
# tools/Makefile             Created   : 2004-05-30
#                            Author    : Alex Tingle
#
#    Copyright (C) 2004 Alex Tingle.
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
#    Modified by REDHAWK (United States Government) - 2015

include ../config.mk

SOURCE_FILES = \
 eventc.cc \
 eventf.cc \
 eventl.cc \
 rmeventc.cc \
 rmeventall.cc \

ifeq ($(CORBA_ORB),omniORB4)
  SOURCE_FILES += events.cc
endif

EXE_FILES = $(patsubst %.cc,%$(EXEEXT),$(SOURCE_FILES))
ARCHIVES  = ../src/naming.$(OBJEXT) ../src/getopt.$(OBJEXT) ../idl/$(CLIENT_LIB)

all: $(EXE_FILES)

install: $(EXE_FILES)
	$(INSTALL) -d $(INSTALL_BIN)
	$(INSTALL) eventc$(EXEEXT)   $(INSTALL_BIN)
	$(INSTALL) eventf$(EXEEXT)   $(INSTALL_BIN)
	$(INSTALL) events$(EXEEXT)   $(INSTALL_BIN)
	$(INSTALL) eventl$(EXEEXT)   $(INSTALL_BIN)
	$(INSTALL) rmeventc$(EXEEXT) $(INSTALL_BIN)
	$(INSTALL) rmeventall$(EXEEXT) $(INSTALL_BIN)

uninstall:
	$(RM) $(patsubst %,$(INSTALL_BIN)/%,$(EXE_FILES))

%$(EXEEXT): %.$(OBJEXT)
	$(call CxxBuildExecutable,$@,$^ $(ARCHIVES))

clean:
	$(RM) $(EXE_FILES) *.$(OBJEXT)

include deps.mk

.PHONY: all install uninstall clean
