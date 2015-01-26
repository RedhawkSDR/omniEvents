#                            Package   : omniEvents
# examples/java/Makefile     Created   : 2004/03/12
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

SOURCE_FILES := \
 Eventc.java \
 PushCons.java \
 PushSupp.java \

include ../../config.mk

TARGETS := $(patsubst %.java,%.class,$(SOURCE_FILES))

IDL_FILES := \
 EventChannelAdmin.idl \

IDL_COS_FILES := \
 CosLifeCycle.idl \
 CosEventComm.idl \
 CosEventChannelAdmin.idl \
 
IDL_COS_TARGETS := $(patsubst %.idl,org/omg/%,$(IDL_COS_FILES))
IDL_TARGETS     := $(patsubst %.idl,net/sourceforge/omniorb/%,$(IDL_FILES))

IDLJ := idlj -fclient -fserver -i $(IDL_COS_DIR)/COS

all: $(TARGETS)

$(TARGETS): %.class: %.java compile_idl_files
	javac $<

compile_idl_files: $(IDL_COS_TARGETS) $(IDL_TARGETS)

$(IDL_COS_TARGETS): org/omg/%: $(IDL_COS_DIR)/COS/%.idl
	$(IDLJ) $^

$(IDL_TARGETS): net/sourceforge/omniorb/%: $(OMNIEVENTS_BASE)/idl/%.idl
	$(IDLJ) $^

clean:
	$(RMDIR) org
	$(RMDIR) net
	$(RM) *.class

.PHONY: all compile_idl_files clean
