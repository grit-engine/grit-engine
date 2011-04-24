# (c) David Cunningham 2011, Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

# this Makefile is just used to set up some variables, the muscle is in
# ../src/common.mk which is used by both linux and win32 builds

ROOT=../..

include $(ROOT)/common.mk


TARGETS=extract colread imgread ideread iplread ifpread dffread csvread handling surfinfo procobj


all: $(TARGETS)

../../grit_core/linux/TColLexer.o:
	@echo "Run make in grit_core/linux first!" >&2
	@false
../../grit_core/linux/TColParser.o:
	@echo "Run make in grit_core/linux first!" >&2
	@false
../../grit_core/linux/TColLexer-core-engine.o:
	@echo "Run make in grit_core/linux first!" >&2
	@false

TColLexer.o: ../../grit_core/linux/TColLexer.o
	ln -sf "$^" "$@"

TColParser.o: ../../grit_core/linux/TColParser.o
	ln -sf "$^" "$@"

TColLexer-core-engine.o: ../../grit_core/linux/TColLexer-core-engine.o
	ln -sf "$^" "$@"

depend:
	makedepend -Y -f ../src/common.mk -I ../../grit_core/src ../src/*.c++
	sed -i 's_^[.][.]/src/\([^.]*.o:\)_\1_g' ../src/common.mk

include ../src/common.mk

# vim: shiftwidth=8:tabstop=8:noexpandtab
