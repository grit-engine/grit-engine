# this Makefile is just used to set up some variables, the muscle is in
# ../src/common.mk which is used by both linux and win32 builds

ROOT=../..

include $(ROOT)/common.mk

TARGETS=grit.x11 matbin

grit: grit.x11 grit.x11.stripped

%.stripped: %
	strip -s $< -o $@

all: $(TARGETS)

include ../src/common.mk

# vim: ts=8:sw=8:noexpandtab
