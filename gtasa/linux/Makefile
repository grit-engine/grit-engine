# (c) David Cunningham 2011, Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

ROOT=../..

include $(ROOT)/common.mk


TARGETS=extract colread imgread ideread iplread ifpread dffread csvread handling surfinfo procobj

all: $(TARGETS)

CXXFLAGS += $(GRIT_BASE_CXXFLAGS) $(OGRE_CXXFLAGS) $(QUEX_CXXFLAGS)
LDFLAGS  += $(GRIT_BASE_LDFLAGS) $(OGRE_LDFLAGS) $(QUEX_LDFLAGS)
LDLIBS   += $(GRIT_BASE_LDLIBS) $(OGRE_LDLIBS) $(QUEX_LDLIBS)

ALL_OPTS=$(CXXFLAGS) $(LDFLAGS) $(LDLIBS)



../../grit_core/linux/TColLexer.o ../../grit_core/linux/TColParser.o ../../grit_core/linux/TColLexer-core-engine.o ../../grit_core/linux/BColParser.o:
	@echo "Run make in grit_core/linux first!" >&2
	@false


%.o: ../../grit_core/linux/%.o
	ln -sf "$<" "$@"


%.o: ../src/%.cpp
	@$(COMPILING)
	@$(CXX) -pedantic -c $< -o $@ $(CXXFLAGS)

imgread: ../src/imgread.cpp 
	@$(LINKING)
	@$(CXX) -pedantic -D_IMGREAD_EXEC $^ -o $@ $(ALL_OPTS)

ifpread: ../src/ifpread.cpp
	@$(LINKING)
	@$(CXX) -pedantic -D_IFPREAD_EXEC $^ -o $@ $(ALL_OPTS)

extract: ../src/extract.cpp ColParser.o BColParser.o TColParser.o ideread.o imgread.o dffread.o tex_dups.o iplread.o txdread.o dirutil.o csvread.o handling.o surfinfo.o procobj.o
	@$(LINKING)
	@$(CXX) -pedantic $^ -o $@ $(ALL_OPTS)

ideread: ../src/ideread.cpp csvread.o
	@$(LINKING)
	@$(CXX) -pedantic -D_IDEREAD_EXEC $^ -o $@ $(ALL_OPTS)

dffread: ../src/dffread.cpp tex_dups.o ColParser.o TColParser.o
	@$(LINKING)
	@$(CXX) -pedantic -D_DFFREAD_EXEC $^ -o $@ $(ALL_OPTS)

txdread: ../src/txdread.cpp
	@$(LINKING)
	@$(CXX) -pedantic -D_TXDREAD_EXEC $^ -o $@ $(ALL_OPTS)

iplread: ../src/iplread.cpp csvread.o
	@$(LINKING)
	@$(CXX) -pedantic -D_IPLREAD_EXEC $^ -o $@ $(ALL_OPTS)

colread: colread.o ColParser.o TColParser.o TColLexer.o TColLexer-core-engine.o 
	@$(LINKING)
	@$(CXX) -pedantic $^ -o $@ $(ALL_OPTS)

csvread: ../src/csvread.cpp
	@$(LINKING)
	@$(CXX) -pedantic -D_CSVREAD_EXEC $^ -o $@ $(ALL_OPTS)

handling: ../src/handling.cpp csvread.o
	@$(LINKING)
	@$(CXX) -pedantic -D_HANDLING_EXEC $^ -o $@ $(ALL_OPTS)

surfinfo: ../src/surfinfo.cpp csvread.o
	@$(LINKING)
	@$(CXX) -pedantic -D_SURFINFO_EXEC $^ -o $@ $(ALL_OPTS)

procobj: ../src/procobj.cpp csvread.o
	@$(LINKING)
	@$(CXX) -pedantic -D_PROCOBJ_EXEC $^ -o $@ $(ALL_OPTS)


clean:
	rm -fv $(TARGETS) *.o


depend:
	makedepend -Y -f Makefile -I ../../grit_core/src -I ../depend_stubs ../src/*.c++
	sed -i 's_^[.][.]/src/\([^.]*.o:\)_\1_g' Makefile



# vim: shiftwidth=8:tabstop=8:noexpandtab
