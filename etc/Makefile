#                            Package   : omniEvents
# etc/Makefile               Created   : 2004/3/31
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

include ../config.mk

# The init script is installed in /etc/init.d or /sbin/init.d, so long as that
# directory ALREADY EXISTS.
# Exception: If DESTDIR is set, then always install.
ifndef DESTDIR
INSTALLCHECK  := installcheck
endif

# The config file is installed in /etc/default or /etc/sysconfig. An existing
# config file is never overwritten or uninstalled.

SERVICE_NAME  := omniorb-eventservice

all:

installcheck:
	test -d $(INSTALL_ETC)/init.d || \
	  ( echo "No such directory: $(INSTALL_ETC)/init.d" ; exit 1 )

install: $(INSTALLCHECK)
	$(INSTALL) -d $(INSTALL_ETC)/init.d
	$(INSTALL) -d $(SYSCONFIG_DIR)
	$(INSTALL) -m 744 init.d/$(SERVICE_NAME) $(INSTALL_ETC)/init.d
	test -f $(SYSCONFIG_DIR)/$(SERVICE_NAME) || \
	  $(INSTALL) -m 644 default/$(SERVICE_NAME) $(SYSCONFIG_DIR)

uninstall:
	# Don't uninstall config file.
	$(RM) $(INSTALL_ETC)/init.d/$(SERVICE_NAME)

clean:

.PHONY: all installcheck install uninstall clean
