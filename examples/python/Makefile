#                            Package   : omniEvents
# examples/python/Makefile   Created   : 2003/11/16
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

include ../../config.mk

# Set this line if your omniidl complains about...
# "omniidl: Could not import back-end 'python'"
#BACKEND = -pL:\win32\src\omniORBpy-2.2\lib\python

OMNIEVENTS_BASE=../..

IDL_FILES = \
 $(OMNIEVENTS_BASE)/idl/EventChannelAdmin.idl \

IDL_COS_FILES = \
 CosLifeCycle.idl \
 CosEventComm.idl \
 CosEventChannelAdmin.idl \
 CosTypedEventComm.idl \
 CosTypedEventChannelAdmin.idl \

all: compile_idl_files

compile_idl_files:
	$(IDL) -bpython $(BACKEND) -I$(IDL_COS_DIR)/COS $(patsubst %,$(IDL_COS_DIR)/COS/%,$(IDL_COS_FILES))
	$(IDL) -bpython $(BACKEND) -I$(IDL_COS_DIR)/COS $(IDL_FILES)

install:

clean:
	$(RMDIR) EventChannelAdmin* Cos* *.pyc

.PHONY: all compile_idl_files install clean
