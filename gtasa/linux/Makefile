# this Makefile is just used to set up some variables, the muscle is in
# ../src/common.mk which is used by both linux and win32 builds

ROOT=../..

include $(ROOT)/common.mk


TARGETS=extract colread imgread ideread iplread ifpread


all: $(TARGETS)

../../grit_core/linux/TColLexer.o:
	@echo "Run make in grit_core/linux first!" >&2
	false
../../grit_core/linux/TColParser.o:
	@echo "Run make in grit_core/linux first!" >&2
	false
../../grit_core/linux/TColLexer-core-engine.o:
	@echo "Run make in grit_core/linux first!" >&2
	false

TColLexer.o: ../../grit_core/linux/TColLexer.o
	ln -sf "$^" "$@"

TColParser.o: ../../grit_core/linux/TColParser.o
	ln -sf "$^" "$@"

TColLexer-core-engine.o: ../../grit_core/linux/TColLexer-core-engine.o
	ln -sf "$^" "$@"

include ../src/common.mk

# vim: shiftwidth=8:tabstop=8:noexpandtab
