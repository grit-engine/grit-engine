# (c) David Cunningham 2011, Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

# this Makefile is just used to set up some variables, the muscle is in
# ../src/common.mk which is used by both linux and win32 builds

ROOT=../..

include $(ROOT)/common.mk

TARGETS=grit.x11 matbin

grit: ../media/Grit.x11 grit.x11.stripped

%.stripped: %
	strip -s $< -o $@

../media/Grit.x11: grit.x11.stripped
	cp $< $@

all: $(TARGETS)

depend:
	makedepend -Y -f ../src/common.mk ../src/*.c++ ../src/linux/*.c++ ../src/win32/*.c++
	sed -i 's_^[.][.]/src/\(\(linux\|win32\)/\)\?\([^.]*[.]o:\)_\3_g' ../src/common.mk

.PHONY: depend

include ../src/common.mk

# vim: ts=8:sw=8:noexpandtab
