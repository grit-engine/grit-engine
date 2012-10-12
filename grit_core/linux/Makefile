# (c) David Cunningham and the Grit Game Engine project 2012
# Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

# this Makefile is just used to set up some variables, the muscle is in
# ../src/common.mk which is used by both linux and win32 builds

ROOT=../..

include $(ROOT)/common.mk

GRIT_BLENDER_DIR=$(ROOT)/exporters/blender_scripts/addons/grit_blender

TARGETS=$(ROOT)/grit_core/media/Grit.linux.x86 \
        Grit.linux.x86.stripped \
        $(GRIT_BLENDER_DIR)/grit_col_conv \

all: $(TARGETS)

%.stripped: %
	strip -s $< -o $@

$(ROOT)/grit_core/media/Grit.linux.x86: Grit.linux.x86.stripped
	cp -f $< $@

$(GRIT_BLENDER_DIR)/grit_col_conv: grit_col_conv
	cp -f grit_col_conv $@


depend:
	makedepend -Y -f ../src/common.mk ../src/*.cpp ../src/linux/*.cpp ../src/win32/*.cpp ../src/gfx/*.cpp ../src/physics/*.cpp ../src/audio/*.cpp -I ../depend_stubs
	sed -i 's_^[.][.]/src/linux/\([^.]*[.]o:\)_\1_g' ../src/common.mk
	sed -i 's_^[.][.]/src/win32/\([^.]*[.]o:\)_\1_g' ../src/common.mk
	sed -i 's_^[.][.]/src/gfx/\([^.]*[.]o:\)_\1_g' ../src/common.mk
	sed -i 's_^[.][.]/src/physics/\([^.]*[.]o:\)_\1_g' ../src/common.mk
	sed -i 's_^[.][.]/src/audio/\([^.]*[.]o:\)_\1_g' ../src/common.mk
	sed -i 's_^[.][.]/src/\([^.]*[.]o:\)_\1_g' ../src/common.mk


.PHONY: depend

include ../src/common.mk

# vim: ts=8:sw=8:noexpandtab
