# (c) David Cunningham 2011, Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

ROOT=../..

include $(ROOT)/common.mk


TARGETS=extract colread imgread ideread iplread ifpread dffread csvread handling surfinfo procobj txdread

all: $(TARGETS)

CXXFLAGS += $(GRIT_BASE_CXXFLAGS) $(OGRE_CXXFLAGS) $(QUEX_CXXFLAGS) -I $(ROOT)/dependencies/util
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
	makedepend -Y -f Makefile -I ../../grit_core/src -I ../depend_stubs ../src/*.cpp
	sed -i 's_^[.][.]/src/\([^.]*.o:\)_\1_g' Makefile



# DO NOT DELETE

ColParser.o: ../depend_stubs/locale ../depend_stubs/algorithm
ColParser.o: ../depend_stubs/iostream ../depend_stubs/fstream
ColParser.o: ../src/ColParser.h
ColParser.o: ../../grit_core/src/physics/TColParser.h
ColParser.o: ../depend_stubs/string ../src/ios_util.h
ColParser.o: ../depend_stubs/sstream ../depend_stubs/cstdlib
ColParser.o: ../depend_stubs/cstring ../depend_stubs/cerrno
ColParser.o: ../../grit_core/src/portable_io.h ../depend_stubs/cstdio
ColParser.o: ../depend_stubs/cassert ../depend_stubs/stdint.h
ColParser.o: ../../grit_core/src/CentralisedLog.h
ColParser.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
colread.o: ../depend_stubs/cstdlib ../depend_stubs/cstring
colread.o: ../depend_stubs/cerrno ../depend_stubs/iostream
colread.o: ../depend_stubs/fstream ../src/ios_util.h
colread.o: ../depend_stubs/sstream ../../grit_core/src/portable_io.h
colread.o: ../depend_stubs/cstdio ../depend_stubs/cassert
colread.o: ../depend_stubs/string ../depend_stubs/stdint.h
colread.o: ../../grit_core/src/CentralisedLog.h
colread.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
colread.o: ../../grit_core/src/physics/TColParser.h ../src/ColParser.h
csvread.o: ../depend_stubs/iostream ../depend_stubs/fstream
csvread.o: ../src/csvread.h ../depend_stubs/istream
csvread.o: ../depend_stubs/vector ../depend_stubs/string
csvread.o: ../depend_stubs/map ../src/ios_util.h
csvread.o: ../depend_stubs/sstream ../depend_stubs/cstdlib
csvread.o: ../depend_stubs/cstring ../depend_stubs/cerrno
csvread.o: ../../grit_core/src/portable_io.h ../depend_stubs/cstdio
csvread.o: ../depend_stubs/cassert ../depend_stubs/stdint.h
csvread.o: ../../grit_core/src/CentralisedLog.h
csvread.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
dffread.o: ../depend_stubs/cstdlib ../depend_stubs/climits
dffread.o: ../depend_stubs/locale ../depend_stubs/map
dffread.o: ../depend_stubs/string ../depend_stubs/vector
dffread.o: ../depend_stubs/sstream ../depend_stubs/iostream
dffread.o: ../depend_stubs/fstream
dffread.o: ../depend_stubs/OgreDefaultHardwareBufferManager.h
dffread.o: ../depend_stubs/OgreMesh.h ../depend_stubs/OgreSubMesh.h
dffread.o: ../depend_stubs/OgreMeshSerializer.h
dffread.o: ../depend_stubs/OgreLogManager.h
dffread.o: ../depend_stubs/OgreResourceGroupManager.h
dffread.o: ../depend_stubs/OgreMeshManager.h
dffread.o: ../depend_stubs/OgreLodStrategyManager.h ../src/ios_util.h
dffread.o: ../depend_stubs/cstring ../depend_stubs/cerrno
dffread.o: ../../grit_core/src/portable_io.h ../depend_stubs/cstdio
dffread.o: ../depend_stubs/cassert ../depend_stubs/stdint.h
dffread.o: ../../grit_core/src/CentralisedLog.h
dffread.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
dffread.o: ../src/dffread.h ../depend_stubs/set ../src/ideread.h
dffread.o: ../depend_stubs/istream
dffread.o: ../../grit_core/src/physics/TColParser.h ../src/ColParser.h
dffread.o: ../src/tex_dups.h
dirutil.o: ../depend_stubs/cerrno ../depend_stubs/sys/types.h
dirutil.o: ../depend_stubs/sys/stat.h ../depend_stubs/unistd.h
dirutil.o: ../src/dirutil.h ../depend_stubs/string ../src/ios_util.h
dirutil.o: ../depend_stubs/sstream ../depend_stubs/iostream
dirutil.o: ../depend_stubs/cstdlib ../depend_stubs/cstring
dirutil.o: ../../grit_core/src/portable_io.h ../depend_stubs/cstdio
dirutil.o: ../depend_stubs/cassert ../depend_stubs/fstream
dirutil.o: ../depend_stubs/stdint.h
dirutil.o: ../../grit_core/src/CentralisedLog.h
dirutil.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
extract.o: ../depend_stubs/cstdlib ../depend_stubs/cmath
extract.o: ../depend_stubs/iostream ../depend_stubs/fstream
extract.o: ../depend_stubs/locale ../depend_stubs/algorithm
extract.o: ../src/imgread.h ../depend_stubs/string
extract.o: ../depend_stubs/vector ../depend_stubs/map ../src/iplread.h
extract.o: ../src/ideread.h ../depend_stubs/istream ../src/tex_dups.h
extract.o: ../src/dffread.h ../depend_stubs/set
extract.o: ../../grit_core/src/physics/TColParser.h ../src/ColParser.h
extract.o: ../src/txdread.h ../../grit_core/src/physics/BColParser.h
extract.o: ../depend_stubs/stdint.h ../src/ios_util.h
extract.o: ../depend_stubs/sstream ../depend_stubs/cstring
extract.o: ../depend_stubs/cerrno ../../grit_core/src/portable_io.h
extract.o: ../depend_stubs/cstdio ../depend_stubs/cassert
extract.o: ../../grit_core/src/CentralisedLog.h
extract.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
extract.o: ../src/dirutil.h ../src/csvread.h ../src/handling.h
extract.o: ../src/surfinfo.h ../src/procobj.h
handling.o: ../depend_stubs/cstdlib ../depend_stubs/iostream
handling.o: ../depend_stubs/fstream ../depend_stubs/algorithm
handling.o: ../depend_stubs/locale ../src/handling.h
handling.o: ../depend_stubs/istream ../depend_stubs/vector
handling.o: ../depend_stubs/string ../depend_stubs/map
handling.o: ../src/csvread.h ../src/ios_util.h ../depend_stubs/sstream
handling.o: ../depend_stubs/cstring ../depend_stubs/cerrno
handling.o: ../../grit_core/src/portable_io.h ../depend_stubs/cstdio
handling.o: ../depend_stubs/cassert ../depend_stubs/stdint.h
handling.o: ../../grit_core/src/CentralisedLog.h
handling.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
ideread.o: ../depend_stubs/climits ../depend_stubs/cmath
ideread.o: ../depend_stubs/cerrno ../depend_stubs/iostream
ideread.o: ../depend_stubs/string ../depend_stubs/sstream
ideread.o: ../depend_stubs/vector ../depend_stubs/fstream
ideread.o: ../depend_stubs/istream ../depend_stubs/iomanip
ideread.o: ../depend_stubs/algorithm ../depend_stubs/locale
ideread.o: ../src/ideread.h ../src/ios_util.h ../depend_stubs/cstdlib
ideread.o: ../depend_stubs/cstring ../../grit_core/src/portable_io.h
ideread.o: ../depend_stubs/cstdio ../depend_stubs/cassert
ideread.o: ../depend_stubs/stdint.h
ideread.o: ../../grit_core/src/CentralisedLog.h
ideread.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
ideread.o: ../src/csvread.h ../depend_stubs/map
ifpread.o: ../depend_stubs/cstdlib ../depend_stubs/cmath
ifpread.o: ../depend_stubs/sstream ../depend_stubs/string
ifpread.o: ../depend_stubs/vector ../depend_stubs/iostream
ifpread.o: ../depend_stubs/fstream ../src/ios_util.h
ifpread.o: ../depend_stubs/cstring ../depend_stubs/cerrno
ifpread.o: ../../grit_core/src/portable_io.h ../depend_stubs/cstdio
ifpread.o: ../depend_stubs/cassert ../depend_stubs/stdint.h
ifpread.o: ../../grit_core/src/CentralisedLog.h
ifpread.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
imgread.o: ../depend_stubs/string ../depend_stubs/vector
imgread.o: ../depend_stubs/fstream ../depend_stubs/algorithm
imgread.o: ../depend_stubs/locale ../src/ios_util.h
imgread.o: ../depend_stubs/sstream ../depend_stubs/iostream
imgread.o: ../depend_stubs/cstdlib ../depend_stubs/cstring
imgread.o: ../depend_stubs/cerrno ../../grit_core/src/portable_io.h
imgread.o: ../depend_stubs/cstdio ../depend_stubs/cassert
imgread.o: ../depend_stubs/stdint.h
imgread.o: ../../grit_core/src/CentralisedLog.h
imgread.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
imgread.o: ../src/imgread.h ../depend_stubs/map
iplread.o: ../depend_stubs/cmath ../depend_stubs/cerrno
iplread.o: ../depend_stubs/string ../depend_stubs/sstream
iplread.o: ../depend_stubs/vector ../depend_stubs/iostream
iplread.o: ../depend_stubs/fstream ../depend_stubs/algorithm
iplread.o: ../depend_stubs/locale ../src/iplread.h ../src/ios_util.h
iplread.o: ../depend_stubs/cstdlib ../depend_stubs/cstring
iplread.o: ../../grit_core/src/portable_io.h ../depend_stubs/cstdio
iplread.o: ../depend_stubs/cassert ../depend_stubs/stdint.h
iplread.o: ../../grit_core/src/CentralisedLog.h
iplread.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
iplread.o: ../src/csvread.h ../depend_stubs/istream
iplread.o: ../depend_stubs/map
procobj.o: ../depend_stubs/iostream ../depend_stubs/fstream
procobj.o: ../src/procobj.h ../depend_stubs/istream
procobj.o: ../depend_stubs/vector ../depend_stubs/string
procobj.o: ../depend_stubs/map ../src/csvread.h ../src/ios_util.h
procobj.o: ../depend_stubs/sstream ../depend_stubs/cstdlib
procobj.o: ../depend_stubs/cstring ../depend_stubs/cerrno
procobj.o: ../../grit_core/src/portable_io.h ../depend_stubs/cstdio
procobj.o: ../depend_stubs/cassert ../depend_stubs/stdint.h
procobj.o: ../../grit_core/src/CentralisedLog.h
procobj.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
surfinfo.o: ../depend_stubs/iostream ../depend_stubs/fstream
surfinfo.o: ../src/surfinfo.h ../depend_stubs/istream
surfinfo.o: ../depend_stubs/vector ../depend_stubs/string
surfinfo.o: ../depend_stubs/map ../src/csvread.h ../src/ios_util.h
surfinfo.o: ../depend_stubs/sstream ../depend_stubs/cstdlib
surfinfo.o: ../depend_stubs/cstring ../depend_stubs/cerrno
surfinfo.o: ../../grit_core/src/portable_io.h ../depend_stubs/cstdio
surfinfo.o: ../depend_stubs/cassert ../depend_stubs/stdint.h
surfinfo.o: ../../grit_core/src/CentralisedLog.h
surfinfo.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
tex_dups.o: ../depend_stubs/cstdlib ../depend_stubs/iostream
tex_dups.o: ../depend_stubs/string ../src/tex_dups.h
tex_dups.o: ../depend_stubs/map
txdread.o: ../depend_stubs/fstream ../depend_stubs/string
txdread.o: ../depend_stubs/algorithm ../depend_stubs/vector
txdread.o: ../depend_stubs/locale ../src/ios_util.h
txdread.o: ../depend_stubs/sstream ../depend_stubs/iostream
txdread.o: ../depend_stubs/cstdlib ../depend_stubs/cstring
txdread.o: ../depend_stubs/cerrno ../../grit_core/src/portable_io.h
txdread.o: ../depend_stubs/cstdio ../depend_stubs/cassert
txdread.o: ../depend_stubs/stdint.h
txdread.o: ../../grit_core/src/CentralisedLog.h
txdread.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
txdread.o: ../src/txdread.h ../depend_stubs/set
txdread.o: ../depend_stubs/istream
