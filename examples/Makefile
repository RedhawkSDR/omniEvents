#                            Package   : omniEvents
# examples/Makefile          Created   : 2003/10/31
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

SOURCE_FILES = \
 pullcons.cc \
 pullsupp.cc \
 pushcons.cc \
 pushsupp.cc\

include ../config.mk

EXE_FILES = $(patsubst %.cc,%$(EXEEXT),$(SOURCE_FILES))
ARCHIVES  = ../src/naming.$(OBJEXT) ../src/getopt.$(OBJEXT) ../idl/$(CLIENT_LIB)

all: $(EXE_FILES)

%$(EXEEXT): %.$(OBJEXT)
	$(call CxxBuildExecutable,$@,$^ $(ARCHIVES))

clean:
	$(RM) $(EXE_FILES) *.$(OBJEXT) channel


## ----------------------------------------------------------------------------
## This rule builds the 'channel' example, which shows how to use libomniEvents.
##
## It can only be built once libomniEvents has been installed (`make install').
## See the installation instructions in the documentation (or README) for
## how to install libomniEvents.
##
## Once libomniEvents is installed, type `make channel' in this directory
## to build this example program.
##
channel$(EXEEXT): channel.$(OBJEXT)
	$(call CxxBuildExecutable,$@,$^ -lomniEvents)
##
## ----------------------------------------------------------------------------

include deps.mk

.PHONY: all clean
