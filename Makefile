# ============================
# Makefile for building luaimg
# ============================

# Disable  built-in rules, in particular they interfere with C++ headers that have no file
# extension.
.SUFFIXES:


# -----------------------
# User-overrideable parts
# -----------------------

# Override stuff in a user.mk
-include user.mk

CXX?= g++
CC?= gcc
OPT?=-O3 -DNDEBUG
ARCH?= -march=native -mtune=native


# -----------------
# Compilation input
# -----------------

include engine/grit.mk
include gtasa/grit.mk

include dependencies/grit-freeimage/grit.mk
include dependencies/grit-bullet/grit.mk
include dependencies/grit-lua/grit.mk
include dependencies/grit-ogre/grit.mk
include dependencies/grit-util/grit.mk
include dependencies/jsonxx_grit.mk
include dependencies/quex-0.34.1/grit.mk
include dependencies/recastnavigation_grit.mk


PKGCONFIG_DEPS = freetype2 gl glu glew xaw7 zlib zziplib xrandr xrender

# Allow overriding with static deps since this one moves about between distros.
ICU_LDLIBS      = $(shell pkg-config icu-uc icu-i18n --libs)

OPENAL_LDLIBS   ?= -lopenal -lvorbisfile

VORBIS_LDLIBS   ?= -lvorbisfile

GOOGLE_PERF_TOOLS_DEFS ?= USE_GOOGLE_PERF_TOOLS=1
GOOGLE_PERF_TOOLS_LDLIBS ?= -lprofiler



GRIT_WEAK_C_SRCS= \
    $(addprefix dependencies/grit-freeimage/,$(FREEIMAGE_WEAK_C_SRCS)) \
    $(addprefix dependencies/grit-bullet/,$(BULLET_WEAK_C_SRCS)) \
    $(addprefix dependencies/grit-lua/,$(LUA_WEAK_C_SRCS)) \
    $(addprefix dependencies/grit-ogre/,$(OGRE_WEAK_C_SRCS)) \
    $(addprefix dependencies/grit-util/,$(UTIL_WEAK_C_SRCS)) \
    $(addprefix dependencies/jsonxx/,$(JSONXX_WEAK_C_SRCS)) \
    $(addprefix dependencies/quex-0.34.1/,$(QUEX_WEAK_C_SRCS)) \
    $(addprefix dependencies/recastnavigation/,$(RECAST_WEAK_C_SRCS)) \
    $(ICU_WEAK_C_SRCS) \
    $(OPENAL_WEAK_C_SRCS) \
    $(VORBIS_WEAK_C_SRCS) \

GRIT_WEAK_CPP_SRCS= \
    $(addprefix dependencies/grit-freeimage/,$(FREEIMAGE_WEAK_CPP_SRCS)) \
    $(addprefix dependencies/grit-bullet/,$(BULLET_WEAK_CPP_SRCS)) \
    $(addprefix dependencies/grit-lua/,$(LUA_WEAK_CPP_SRCS)) \
    $(addprefix dependencies/grit-ogre/,$(OGRE_WEAK_CPP_SRCS)) \
    $(addprefix dependencies/grit-util/,$(UTIL_WEAK_CPP_SRCS)) \
    $(addprefix dependencies/jsonxx/,$(JSONXX_WEAK_CPP_SRCS)) \
    $(addprefix dependencies/quex-0.34.1/,$(QUEX_WEAK_CPP_SRCS)) \
    $(addprefix dependencies/recastnavigation/,$(RECAST_WEAK_CPP_SRCS)) \
    $(ICU_WEAK_CPP_SRCS) \
    $(OPENAL_WEAK_CPP_SRCS) \
    $(VORBIS_WEAK_CPP_SRCS) \

GRIT_C_SRCS= \
    $(addprefix dependencies/grit-freeimage/,$(FREEIMAGE_C_SRCS)) \
    $(addprefix dependencies/grit-bullet/,$(BULLET_C_SRCS)) \
    $(addprefix dependencies/grit-lua/,$(LUA_C_SRCS)) \
    $(addprefix dependencies/grit-ogre/,$(OGRE_C_SRCS)) \
    $(addprefix dependencies/grit-util/,$(UTIL_C_SRCS)) \
    $(addprefix dependencies/jsonxx/,$(JSONXX_C_SRCS)) \
    $(addprefix dependencies/quex-0.34.1/,$(QUEX_C_SRCS)) \
    $(addprefix dependencies/recastnavigation/,$(RECAST_C_SRCS)) \
    $(ICU_C_SRCS) \
    $(OPENAL_C_SRCS) \
    $(VORBIS_C_SRCS) \

GRIT_CPP_SRCS= \
    $(addprefix dependencies/grit-freeimage/,$(FREEIMAGE_CPP_SRCS)) \
    $(addprefix dependencies/grit-bullet/,$(BULLET_CPP_SRCS)) \
    $(addprefix dependencies/grit-lua/,$(LUA_CPP_SRCS)) \
    $(addprefix dependencies/grit-ogre/,$(OGRE_CPP_SRCS)) \
    $(addprefix dependencies/grit-util/,$(UTIL_CPP_SRCS)) \
    $(addprefix dependencies/jsonxx/,$(JSONXX_CPP_SRCS)) \
    $(addprefix dependencies/quex-0.34.1/,$(QUEX_CPP_SRCS)) \
    $(addprefix dependencies/recastnavigation/,$(RECAST_CPP_SRCS)) \
    $(ICU_CPP_SRCS) \
    $(OPENAL_CPP_SRCS) \
    $(VORBIS_CPP_SRCS) \
	$(addprefix engine/,$(ENGINE_CPP_SRCS)) \


# TODO: remove EXTRACT_INCLUDE_DIRS from here, refactor to put stuff in dependencies/
INCLUDE_DIRS= \
    $(addprefix gtasa/,$(EXTRACT_INCLUDE_DIRS)) \
    $(addprefix dependencies/grit-freeimage/,$(FREEIMAGE_INCLUDE_DIRS)) \
    $(addprefix dependencies/grit-bullet/,$(BULLET_INCLUDE_DIRS)) \
    $(addprefix dependencies/grit-lua/,$(LUA_INCLUDE_DIRS)) \
    $(addprefix dependencies/grit-ogre/,$(OGRE_INCLUDE_DIRS)) \
    $(addprefix dependencies/grit-util/,$(UTIL_INCLUDE_DIRS)) \
    $(addprefix dependencies/jsonxx/,$(JSONXX_INCLUDE_DIRS)) \
    $(addprefix dependencies/quex-0.34.1/,$(QUEX_INCLUDE_DIRS)) \
    $(addprefix dependencies/recastnavigation/,$(RECAST_INCLUDE_DIRS)) \
    $(ICU_INCLUDE_DIRS) \
    $(OPENAL_INCLUDE_DIRS) \
    $(VORBIS_INCLUDE_DIRS) \

CFLAGS= \
	$(INCLUDE_DIRS:%=-isystem%)  \
	$(BULLET_DEFS:%=-D%) \
	$(LUA_DEFS:%=-D%) \
	$(OGRE_DEFS:%=-D%) \
	$(UTIL_DEFS:%=-D%) \
	$(JSONXX_DEFS:%=-D%) \
	$(QUEX_DEFS:%=-D%) \
	$(FREEIMAGE_DEFS:%=-D%) \
	$(ICU_DEFS:%=-D%) \
	$(OPENAL_DEFS:%=-D%) \
	$(VORBIS_DEFS:%=-D%) \
	$(GOOGLE_PERF_TOOLS_DEFS:%=-D%) \
	$(shell pkg-config $(PKGCONFIG_DEPS) --cflags) \

LDFLAGS= \
	$(BULLET_LDFLAGS) \
	$(LUA_LDFLAGS) \
	$(OGRE_LDFLAGS) \
	$(UTIL_LDFLAGS) \
	$(JSONXX_LDFLAGS) \
	$(QUEX_LDFLAGS) \
	$(ICU_LDFLAGS) \
	$(OPENAL_LDFLAGS) \
	$(VORBIS_LDFLAGS) \
	$(shell pkg-config $(PKGCONFIG_DEPS) --libs-only-L --libs-only-other) \

LDLIBS= \
	$(BULLET_LDLIBS) \
	$(LUA_LDLIBS) \
	$(OGRE_LDLIBS) \
	$(UTIL_LDLIBS) \
	$(JSONXX_LDLIBS) \
	$(QUEX_LDLIBS) \
	$(FREEIMAGE_LDLIBS) \
	$(ICU_LDLIBS) \
	$(OPENAL_LDLIBS) \
	$(VORBIS_LDLIBS) \
	$(GOOGLE_PERF_TOOLS_LDLIBS) \
	$(shell pkg-config $(PKGCONFIG_DEPS) --libs-only-l) \
	-lreadline \
	-lm \

COL_CONV_OBJECTS= \
	$(addprefix build/engine/,$(COL_CONV_STANDALONE_CPP_SRCS)) \

EXTRACT_OBJECTS= \
	$(addprefix build/gtasa/,$(EXTRACT_CPP_SRCS)) \
    $(addprefix build/dependencies/grit-freeimage/,$(FREEIMAGE_WEAK_CPP_SRCS:%.cpp=%.weak_cpp)) \
    $(addprefix build/dependencies/grit-freeimage/,$(FREEIMAGE_WEAK_C_SRCS:%.c=%.weak_c)) \
    $(addprefix build/dependencies/grit-freeimage/,$(FREEIMAGE_CPP_SRCS)) \
    $(addprefix build/dependencies/grit-freeimage/,$(FREEIMAGE_C_SRCS)) \
	$(addprefix build/dependencies/grit-ogre/,$(OGRE_WEAK_CPP_SRCS:%.cpp=%.weak_cpp)) \
	$(addprefix build/dependencies/grit-ogre/,$(OGRE_WEAK_C_SRCS:%.c=%.weak_c)) \
	$(addprefix build/dependencies/grit-ogre/,$(OGRE_CPP_SRCS)) \
	$(addprefix build/dependencies/grit-ogre/,$(OGRE_C_SRCS)) \

GRIT_OBJECTS= \
	$(addprefix build/,$(GRIT_WEAK_CPP_SRCS:%.cpp=%.weak_cpp)) \
	$(addprefix build/,$(GRIT_WEAK_C_SRCS:%.c=%.weak_c)) \
	$(addprefix build/,$(GRIT_CPP_SRCS)) \
	$(addprefix build/,$(GRIT_C_SRCS)) \

GSL_OBJECTS= \
	$(addprefix build/engine/,$(GSL_STANDALONE_CPP_SRCS)) \

XMLCONVERTER_OBJECTS= \
    $(addprefix build/dependencies/grit-freeimage/,$(FREEIMAGE_WEAK_CPP_SRCS:%.cpp=%.weak_cpp)) \
    $(addprefix build/dependencies/grit-freeimage/,$(FREEIMAGE_WEAK_C_SRCS:%.c=%.weak_c)) \
    $(addprefix build/dependencies/grit-freeimage/,$(FREEIMAGE_CPP_SRCS)) \
    $(addprefix build/dependencies/grit-freeimage/,$(FREEIMAGE_C_SRCS)) \
	$(addprefix build/dependencies/grit-ogre/,$(XMLCONVERTER_WEAK_CPP_SRCS:%.cpp=%.weak_cpp)) \
	$(addprefix build/dependencies/grit-ogre/,$(XMLCONVERTER_WEAK_C_SRCS:%.c=%.weak_c)) \
	$(addprefix build/dependencies/grit-ogre/,$(XMLCONVERTER_CPP_SRCS)) \
	$(addprefix build/dependencies/grit-ogre/,$(XMLCONVERTER_C_SRCS)) \

ALL_OBJECTS= \
	$(COL_CONV_OBJECTS) \
	$(EXTRACT_OBJECTS) \
	$(GRIT_OBJECTS) \
	$(GSL_OBJECTS) \
	$(XMLCONVERTER_OBJECTS) \

# Caution: -ffast-math broke btContinuousConvexCollision::calcTimeOfImpact, and there seems to be
# no easy way to disable it on a per-file basis.
CODEGEN= \
    $(OPT) \
    $(ARCH) \
	-Winvalid-pch \
    -Wno-type-limits \
    -Wno-deprecated \
    -g \


# -----------
# Build rules
# -----------

PRECOMPILED_HEADER= echo -e '\e[0mPrecompiling header: \e[1;34m$@\e[0m'
COMPUTING_DEPENDENCIES= echo -e '\e[0mComputing dependencies: \e[33m$@\e[0m'
COMPILING= echo -e '\e[0mCompiling: \e[32m$@\e[0m'
LINKING= echo -e '\e[0mLinking: \e[1;32m$@\e[0m'
ALL_EXECUTABLES= extract grit gsl grit_col_conv GritXMLConverter

all: $(ALL_EXECUTABLES)

# Precompiled header
build/stdafx.h.gch: dependencies/stdafx/stdafx.h
	@$(PRECOMPILED_HEADER)
	@mkdir -p $(shell dirname $@)
	@$(CXX) -c $(CODEGEN) -std=c++11 -pedantic $(CFLAGS) $< -o $@
	
build/stdafx.h: dependencies/stdafx/stdafx.h
	cp $< $@
	

build/%.cpp.o: %.cpp build/stdafx.h build/stdafx.h.gch
	@$(COMPILING)
	@mkdir -p $(shell dirname $@)
	@$(CXX) -c $(CODEGEN) -std=c++11 -pedantic -Wall -Wextra -include build/stdafx.h $(CFLAGS) $< -o $@

build/%.c.o: %.c
	@$(COMPILING)
	@mkdir -p $(shell dirname $@)
	@$(CC) -c $(CODEGEN) -std=c99 -pedantic -Wall -Wextra $(CFLAGS) $< -o $@

build/%.weak_cpp.o: %.cpp
	@$(COMPILING)
	@mkdir -p $(shell dirname $@)
	@$(CXX) -c $(CODEGEN) -std=c++11 -Wno-narrowing -Wno-literal-suffix -Wno-unused-result -Wno-format -Wno-terminate -Wno-write-strings -Wno-deprecated-declarations -Wno-pedantic -Wno-int-to-pointer-cast $(CFLAGS) $< -o $@

# Hack to also support dependencies that use .cc
build/%.weak_cpp.o: %.cc
	@$(COMPILING)
	@mkdir -p $(shell dirname $@)
	@$(CXX) -c $(CODEGEN) -std=c++11 $(CFLAGS) $< -o $@

build/%.weak_c.o: %.c
	@$(COMPILING)
	@mkdir -p $(shell dirname $@)
	@$(CC) -c $(CODEGEN) -Wno-implicit-function-declaration $(CFLAGS) $< -o $@

extract: $(addsuffix .o,$(EXTRACT_OBJECTS))
	@$(LINKING)
	@$(CXX) $^ $(LDFLAGS) $(LDLIBS) -o $@

grit: $(addsuffix .o,$(GRIT_OBJECTS))
	@$(LINKING)
	@$(CXX) $^ $(LDFLAGS) $(LDLIBS) -o $@

gsl: $(addsuffix .o,$(GSL_OBJECTS))
	@$(LINKING)
	@$(CXX) $^ $(LDFLAGS) $(LDLIBS) -o $@

grit_col_conv: $(addsuffix .o,$(COL_CONV_OBJECTS))
	@$(LINKING)
	@$(CXX) $^ $(LDFLAGS) $(LDLIBS) -o $@

GritXMLConverter: $(addsuffix .o,$(XMLCONVERTER_OBJECTS))
	@$(LINKING)
	@$(CXX) $^ $(LDFLAGS) $(LDLIBS) -o $@


# ---------
# Dev stuff
# ---------

# Note the two -MT are to ensure that if any of the discovered headers change, the dependencies will
# be re-computed.  This avoids a problem where if you include a new header from an existing header,
# the .d would never be updated and the build would be wrong.

build/stdafx.h.gch.d: dependencies/stdafx/stdafx.h
	@$(COMPUTING_DEPENDENCIES)
	@mkdir -p $(shell dirname $@)
	@$(CXX) -M -std=c++11 $(CFLAGS) $< -o $@ -MT $(@:%.d=%) -MT $@
	
build/%.cpp.d: %.cpp
	@$(COMPUTING_DEPENDENCIES)
	@mkdir -p $(shell dirname $@)
	@$(CXX) -M -std=c++11 $(CFLAGS) $< -o $@ -MT $(@:%.d=%.o) -MT $@

build/%.c.d: %.c
	@$(COMPUTING_DEPENDENCIES)
	@mkdir -p $(shell dirname $@)
	@$(CC) -M -std=c99 $(CFLAGS) $< -o $@ -MT $(@:%.d=%.o) -MT $@

build/%.weak_cpp.d: %.cpp
	@$(COMPUTING_DEPENDENCIES)
	@mkdir -p $(shell dirname $@)
	@$(CXX) -M -std=c++11 $(CFLAGS) $< -o $@ -MT $(@:%.d=%.o) -MT $@

# Hack to also support dependencies that use .cc
build/%.weak_cpp.d: %.cc
	@$(COMPUTING_DEPENDENCIES)
	@mkdir -p $(shell dirname $@)
	@$(CXX) -M -std=c++11 $(CFLAGS) $< -o $@ -MT $(@:%.d=%.o) -MT $@

build/%.weak_c.d: %.c
	@$(COMPUTING_DEPENDENCIES)
	@mkdir -p $(shell dirname $@)
	@$(CC) -M $(CFLAGS) $< -o $@ -MT $(@:%.d=%.o) -MT $@

ALL_DEPS = $(addsuffix .d,$(ALL_OBJECTS)) build/stdafx.h.gch.d

clean_depend:
	@rm -f $(ALL_DEPS)
	@echo Dependencies cleaned.

clean:
	rm -rfv $(ALL_EXECUTABLES) build

-include $(ALL_DEPS)
