#                            Package   : omniEvents
# meta.mk                    Created   : 2003/11/01
#                            Author    : Alex Tingle
#
#    Copyright (C) 2003..2005 Alex Tingle.
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
# Description:
#
#  A rag-bag of meta-actions.
#
#  o  `make -f meta.mk deps' - Update all deps.mk files.
#
#  o  `make -f meta.mk src' - Builds a 'release' tarball that contains all
#          non-generated files (and */deps.mk - for convenience).
#
#  o  `make -f meta.mk win32' - Builds a 'win32' zip that contains binary win32
#          files and library headers.
#

include config.mk

DIRNAME   := $(shell basename `pwd`)

SUBDIRS   := idl src tools examples
DEP_FILES := $(patsubst %,%/deps.mk,$(SUBDIRS))

TAR               = cd ..; tar zcvf $(DIRNAME)-$@.tar.gz -X xfiles $(DIRNAME)
MAKEDEPS          := python $(OMNIEVENTS_BASE)/auto/makedeps.py
MAKEDEPS_INCLUDES := $(filter-out -I$(prefix)/include,$(filter -I%,$(CPPFLAGS)))
MAKEDEPS_FLAGS    := $(MAKEDEPS_INCLUDES) --exclude=CosNaming

EXCLUDE_FROM_RELEASE :=\
 */.* *.bak *.o *.out *.so *.a *.rej *.orig *.pc *~ \
 */CVS* \
 */autom4te.cache* \
 */templates.d* \
 */aclocal.m4 \
 */config.log \
 */config.status \
 *_SAVE \
 */etc/init.d/omniorb-eventservice

deps: compile_idl_files cleandeps $(DEP_FILES)

cleandeps:
	rm -f $(DEP_FILES)

compile_idl_files:
	make -C idl compile_idl_files

$(DEP_FILES):
	echo "Remaking dependencies in $(@D)"
	cd $(@D) ; $(MAKEDEPS) $(MAKEDEPS_FLAGS) *.cc > $(@F)

src: deps doc
	make clean
	mv config.mk config.mk_SAVE; mv src/config.h config.h_SAVE
	cp win32/config.mk config.mk; cp win32/config.h src/config.h
	echo "$(EXCLUDE_FROM_RELEASE)" | tr ' ' '\n' > ../xfiles
	$(TAR)
	rm -f ../xfiles
	mv config.mk_SAVE config.mk; mv config.h_SAVE src/config.h

.PHONY: deps cleandeps compile_idl_files src

# Builds a win32 binary release.
# Precondition: make must already have been run.

FindFiles = $(foreach ext, $(2), $(wildcard $(1)/*.$(ext)))

WIN32_TEXT_FILES = README LICENSE $(wildcard CHANGES*) examples/python/README
WIN32_FILES = \
 $(patsubst %,%.txt,$(WIN32_TEXT_FILES)) \
 omniEvents.exe \
 $(call FindFiles,doc,html css) \
 $(call FindFiles,tools,exe) \
 $(call FindFiles,examples,hh cc exe) \
 $(call FindFiles,examples/python,py) \
 $(call FindFiles,examples/python/*,py) \
 $(call FindFiles,idl,hh cc idl lib) \

.PHONY: win32
win32: README docbook $(DIRNAME) $(patsubst %,$(DIRNAME)/%,$(WIN32_FILES))
	install src/getopt.h src/getopt.cc $(DIRNAME)/examples
	install src/naming.h src/naming.cc $(DIRNAME)/examples
	install src/config.h src/scour.h $(DIRNAME)/examples
	zip -r $(DIRNAME)-$@.zip $(DIRNAME)
	# rm -rf $(DIRNAME)

$(DIRNAME):
	install -d $(DIRNAME)

$(DIRNAME)/%.txt: %
	install -d `dirname $@`
	sed 's/$$//' $< > $@

$(DIRNAME)/omniEvents.exe: src/omniEvents.exe
	install -d `dirname $@`
	install $< $@

$(DIRNAME)/%: %
	install -d `dirname $@`
	install $< $@


##
## DOCUMENTATION
##

doc: doxygen README docbook manpages

doxygen:
	doxygen doc/rc/doxygen.conf

README: doc/omnievents.docbook.xml
	python doc/rc/docbook2text.py $< > $@

XSL := xsltproc
XSLFLAGS := \
  --stringparam make.valid.html 1 \
  --stringparam chunker.output.indent yes \
  --stringparam section.autolabel 1 \
  --stringparam annotate.toc 1 \
  --stringparam generate.section.toc.level 1 \
  --stringparam chunk.first.sections 1 \
  --stringparam html.stylesheet omnievents.docbook.css
BaseDir = --stringparam base.dir $(1)

docbook: doc/index.html
doc/index.html: doc/omnievents.docbook.xml
	$(XSL) $(XSLFLAGS) $(call BaseDir,$(@D)/) $(DOCBOOK)/html/chunk.xsl $<

manpages: doc/man/omniEvents.8
doc/man/omniEvents.8: doc/omnievents.docbook.xml
	cd doc/man; $(XSL) $(DOCBOOK)/manpages/docbook.xsl ../$(<F)


.PHONY: doc doxygen docbook manpages


##
## TESTING
##

DAEMON_EXE := src/$(OMNIEVENTS)$(EXEEXT)

test: $(DAEMON_EXE)
	$(MAKE) -C test $@
	# Does it run at all?
	$(DAEMON_EXE) -V
	# OK, start a server.
	$(DAEMON_EXE) -p58321 -P/tmp/oe.PID -l/tmp -t/tmp/oe.trace -NtestECF
	# Are we going?
	sleep 1 && kill -0 `cat /tmp/oe.PID`
	# Test the Event Channel Factory.
	python test/testeventchannel.py -ORBInitRef 'EventChannelFactory=corbaloc::localhost:58321/omniEvents'
	# Create an event channel.
	tools/eventc$(EXEEXT) -ntestEC -itestEC corbaloc::localhost:58321/omniEvents
	# Test the Event Channel
	python test/test.py -ORBInitRef 'Channel=corbaloc::localhost:58321/testEC'
	# Destroy the channel.
	tools/rmeventc$(EXEEXT) corbaloc::localhost:58321/testEC
	# Are we still going?
	kill -0 `cat /tmp/oe.PID`
	# Stop the server
	kill `cat /tmp/oe.PID`
	# Did it shut down OK?
	sleep 5
	test -f /tmp/oe.PID && echo "FAILED TO SHUTDOWN" || echo "SHUTDOWN OK"

.PHONY: test
