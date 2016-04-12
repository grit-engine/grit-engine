# (c) David Cunningham 2011, Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

ROOT=../..

include $(ROOT)/common.mk


TARGETS=extract colread imgread ideread iplread ifpread dffread csvread handling surfinfo procobj txdread

all: $(TARGETS)

CXXFLAGS += $(GRIT_BASE_CXXFLAGS) $(OGRE_CXXFLAGS) $(QUEX_CXXFLAGS) -I $(ROOT)/dependencies/util -I $(ROOT)/grit_core/src
LDFLAGS  += $(GRIT_BASE_LDFLAGS) $(OGRE_LDFLAGS) $(QUEX_LDFLAGS)
LDLIBS   += $(GRIT_BASE_LDLIBS) $(OGRE_LDLIBS) $(QUEX_LDLIBS)

ALL_OPTS=$(CXXFLAGS) $(LDFLAGS) $(LDLIBS)



../../grit_core/linux/tcol_lexer.o ../../grit_core/linux/tcol_parser.o ../../grit_core/linux/tcol_lexer-core-engine.o ../../grit_core/linux/bcol_parser.o:
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

extract: ../src/extract.cpp col_parser.o bcol_parser.o tcol_parser.o ideread.o imgread.o dffread.o tex_dups.o iplread.o txdread.o dirutil.o csvread.o handling.o surfinfo.o procobj.o
	@$(LINKING)
	@$(CXX) -pedantic $^ -o $@ $(ALL_OPTS)

ideread: ../src/ideread.cpp csvread.o
	@$(LINKING)
	@$(CXX) -pedantic -D_IDEREAD_EXEC $^ -o $@ $(ALL_OPTS)

dffread: ../src/dffread.cpp tex_dups.o col_parser.o tcol_parser.o
	@$(LINKING)
	@$(CXX) -pedantic -D_DFFREAD_EXEC $^ -o $@ $(ALL_OPTS)

txdread: ../src/txdread.cpp
	@$(LINKING)
	@$(CXX) -pedantic -D_TXDREAD_EXEC $^ -o $@ $(ALL_OPTS)

iplread: ../src/iplread.cpp csvread.o
	@$(LINKING)
	@$(CXX) -pedantic -D_IPLREAD_EXEC $^ -o $@ $(ALL_OPTS)

colread: colread.o col_parser.o tcol_parser.o tcol_lexer.o tcol_lexer-core-engine.o 
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
	makedepend -Y -f Makefile -I ../../dependencies/util -I ../../grit_core/src -I ../depend_stubs ../src/*.cpp
	sed -i 's_^[.][.]/src/\([^.]*.o:\)_\1_g' Makefile



# DO NOT DELETE

col_parser.o: ../depend_stubs/locale ../depend_stubs/algorithm
col_parser.o: ../depend_stubs/iostream ../depend_stubs/fstream
col_parser.o: ../src/col_parser.h
col_parser.o: ../../grit_core/src/physics/tcol_parser.h
col_parser.o: ../depend_stubs/string
col_parser.o: ../../dependencies/util/math_util.h
col_parser.o: ../depend_stubs/cmath ../depend_stubs/cfloat
col_parser.o: ../../grit_core/src/physics/tcol_lexer
col_parser.o: ../depend_stubs/vector ../depend_stubs/map
col_parser.o: ../depend_stubs/istream
col_parser.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
col_parser.o: ../../grit_core/src/physics/tcol_lexer-token_ids
col_parser.o: ../depend_stubs/cstdio
col_parser.o: ../depend_stubs/quex/code_base/Token
col_parser.o: ../depend_stubs/quex/code_base/definitions
col_parser.o: ../depend_stubs/quex/code_base/buffer/Buffer
col_parser.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
col_parser.o: ../depend_stubs/quex/code_base/template/QuexMode
col_parser.o: ../depend_stubs/quex/code_base/template/Analyser
col_parser.o: ../depend_stubs/quex/code_base/template/Counter
col_parser.o: ../depend_stubs/quex/code_base/template/Accumulator
col_parser.o: ../depend_stubs/quex/code_base/TokenQueue
col_parser.o: ../depend_stubs/cstdlib
col_parser.o: ../depend_stubs/quex/code_base/temporary_macros_on
col_parser.o: ../depend_stubs/quex/code_base/temporary_macros_off
col_parser.o: ../depend_stubs/quex/code_base/template/constructor.i
col_parser.o: ../depend_stubs/quex/code_base/template/Counter.i
col_parser.o: ../depend_stubs/quex/code_base/template/Accumulator.i
col_parser.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
col_parser.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
col_parser.o: ../depend_stubs/quex/code_base/template/mode_handling.i
col_parser.o: ../depend_stubs/quex/code_base/template/misc.i
col_parser.o: ../depend_stubs/quex/code_base/template/buffer_access.i
col_parser.o: ../depend_stubs/quex/code_base/template/Analyser.i
col_parser.o: ../src/ios_util.h ../depend_stubs/sstream
col_parser.o: ../depend_stubs/cstring ../depend_stubs/cerrno
col_parser.o: ../../grit_core/src/portable_io.h
col_parser.o: ../depend_stubs/cassert ../depend_stubs/stdint.h
col_parser.o: ../../grit_core/src/centralised_log.h
col_parser.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
col_parser.o: ../../dependencies/util/console.h
col_parser.o: ../../dependencies/util/exception.h
col_parser.o: ../../dependencies/util/intrinsics.h
colread.o: ../depend_stubs/cstdlib ../depend_stubs/cstring
colread.o: ../depend_stubs/cerrno ../depend_stubs/iostream
colread.o: ../depend_stubs/fstream ../src/ios_util.h
colread.o: ../depend_stubs/sstream ../../grit_core/src/portable_io.h
colread.o: ../depend_stubs/cstdio ../depend_stubs/cassert
colread.o: ../depend_stubs/string ../depend_stubs/stdint.h
colread.o: ../../grit_core/src/centralised_log.h
colread.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
colread.o: ../../dependencies/util/console.h
colread.o: ../../dependencies/util/exception.h
colread.o: ../../dependencies/util/intrinsics.h
colread.o: ../../grit_core/src/physics/tcol_parser.h
colread.o: ../../dependencies/util/math_util.h ../depend_stubs/cmath
colread.o: ../depend_stubs/cfloat
colread.o: ../../grit_core/src/physics/tcol_lexer
colread.o: ../depend_stubs/vector ../depend_stubs/map
colread.o: ../depend_stubs/istream
colread.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
colread.o: ../../grit_core/src/physics/tcol_lexer-token_ids
colread.o: ../depend_stubs/quex/code_base/Token
colread.o: ../depend_stubs/quex/code_base/definitions
colread.o: ../depend_stubs/quex/code_base/buffer/Buffer
colread.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
colread.o: ../depend_stubs/quex/code_base/template/QuexMode
colread.o: ../depend_stubs/quex/code_base/template/Analyser
colread.o: ../depend_stubs/quex/code_base/template/Counter
colread.o: ../depend_stubs/quex/code_base/template/Accumulator
colread.o: ../depend_stubs/quex/code_base/TokenQueue
colread.o: ../depend_stubs/quex/code_base/temporary_macros_on
colread.o: ../depend_stubs/quex/code_base/temporary_macros_off
colread.o: ../depend_stubs/quex/code_base/template/constructor.i
colread.o: ../depend_stubs/quex/code_base/template/Counter.i
colread.o: ../depend_stubs/quex/code_base/template/Accumulator.i
colread.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
colread.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
colread.o: ../depend_stubs/quex/code_base/template/mode_handling.i
colread.o: ../depend_stubs/quex/code_base/template/misc.i
colread.o: ../depend_stubs/quex/code_base/template/buffer_access.i
colread.o: ../depend_stubs/quex/code_base/template/Analyser.i
colread.o: ../src/col_parser.h
csvread.o: ../depend_stubs/iostream ../depend_stubs/fstream
csvread.o: ../src/csvread.h ../depend_stubs/istream
csvread.o: ../depend_stubs/vector ../depend_stubs/string
csvread.o: ../depend_stubs/map ../src/ios_util.h
csvread.o: ../depend_stubs/sstream ../depend_stubs/cstdlib
csvread.o: ../depend_stubs/cstring ../depend_stubs/cerrno
csvread.o: ../../grit_core/src/portable_io.h ../depend_stubs/cstdio
csvread.o: ../depend_stubs/cassert ../depend_stubs/stdint.h
csvread.o: ../../grit_core/src/centralised_log.h
csvread.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
csvread.o: ../../dependencies/util/console.h
csvread.o: ../../dependencies/util/exception.h
csvread.o: ../../dependencies/util/intrinsics.h
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
dffread.o: ../../grit_core/src/centralised_log.h
dffread.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
dffread.o: ../../dependencies/util/console.h
dffread.o: ../../dependencies/util/exception.h
dffread.o: ../../dependencies/util/intrinsics.h ../src/dffread.h
dffread.o: ../depend_stubs/set ../src/ideread.h
dffread.o: ../depend_stubs/istream
dffread.o: ../../grit_core/src/physics/tcol_parser.h
dffread.o: ../../dependencies/util/math_util.h ../depend_stubs/cmath
dffread.o: ../depend_stubs/cfloat
dffread.o: ../../grit_core/src/physics/tcol_lexer
dffread.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
dffread.o: ../../grit_core/src/physics/tcol_lexer-token_ids
dffread.o: ../depend_stubs/quex/code_base/Token
dffread.o: ../depend_stubs/quex/code_base/definitions
dffread.o: ../depend_stubs/quex/code_base/buffer/Buffer
dffread.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
dffread.o: ../depend_stubs/quex/code_base/template/QuexMode
dffread.o: ../depend_stubs/quex/code_base/template/Analyser
dffread.o: ../depend_stubs/quex/code_base/template/Counter
dffread.o: ../depend_stubs/quex/code_base/template/Accumulator
dffread.o: ../depend_stubs/quex/code_base/TokenQueue
dffread.o: ../depend_stubs/quex/code_base/temporary_macros_on
dffread.o: ../depend_stubs/quex/code_base/temporary_macros_off
dffread.o: ../depend_stubs/quex/code_base/template/constructor.i
dffread.o: ../depend_stubs/quex/code_base/template/Counter.i
dffread.o: ../depend_stubs/quex/code_base/template/Accumulator.i
dffread.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
dffread.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
dffread.o: ../depend_stubs/quex/code_base/template/mode_handling.i
dffread.o: ../depend_stubs/quex/code_base/template/misc.i
dffread.o: ../depend_stubs/quex/code_base/template/buffer_access.i
dffread.o: ../depend_stubs/quex/code_base/template/Analyser.i
dffread.o: ../src/col_parser.h ../src/tex_dups.h
dirutil.o: ../depend_stubs/cerrno ../depend_stubs/sys/types.h
dirutil.o: ../depend_stubs/sys/stat.h ../depend_stubs/unistd.h
dirutil.o: ../src/dirutil.h ../depend_stubs/string ../src/ios_util.h
dirutil.o: ../depend_stubs/sstream ../depend_stubs/iostream
dirutil.o: ../depend_stubs/cstdlib ../depend_stubs/cstring
dirutil.o: ../../grit_core/src/portable_io.h ../depend_stubs/cstdio
dirutil.o: ../depend_stubs/cassert ../depend_stubs/fstream
dirutil.o: ../depend_stubs/stdint.h
dirutil.o: ../../grit_core/src/centralised_log.h
dirutil.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
dirutil.o: ../../dependencies/util/console.h
dirutil.o: ../../dependencies/util/exception.h
dirutil.o: ../../dependencies/util/intrinsics.h
extract.o: ../depend_stubs/cstdlib ../depend_stubs/cmath
extract.o: ../depend_stubs/iostream ../depend_stubs/fstream
extract.o: ../depend_stubs/locale ../depend_stubs/algorithm
extract.o: ../src/imgread.h ../depend_stubs/string
extract.o: ../depend_stubs/vector ../depend_stubs/map ../src/iplread.h
extract.o: ../src/ideread.h ../depend_stubs/istream ../src/tex_dups.h
extract.o: ../src/dffread.h ../depend_stubs/set
extract.o: ../../grit_core/src/physics/tcol_parser.h
extract.o: ../../dependencies/util/math_util.h ../depend_stubs/cfloat
extract.o: ../../grit_core/src/physics/tcol_lexer
extract.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
extract.o: ../../grit_core/src/physics/tcol_lexer-token_ids
extract.o: ../depend_stubs/cstdio ../depend_stubs/quex/code_base/Token
extract.o: ../depend_stubs/quex/code_base/definitions
extract.o: ../depend_stubs/quex/code_base/buffer/Buffer
extract.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
extract.o: ../depend_stubs/quex/code_base/template/QuexMode
extract.o: ../depend_stubs/quex/code_base/template/Analyser
extract.o: ../depend_stubs/quex/code_base/template/Counter
extract.o: ../depend_stubs/quex/code_base/template/Accumulator
extract.o: ../depend_stubs/quex/code_base/TokenQueue
extract.o: ../depend_stubs/quex/code_base/temporary_macros_on
extract.o: ../depend_stubs/quex/code_base/temporary_macros_off
extract.o: ../depend_stubs/quex/code_base/template/constructor.i
extract.o: ../depend_stubs/quex/code_base/template/Counter.i
extract.o: ../depend_stubs/quex/code_base/template/Accumulator.i
extract.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
extract.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
extract.o: ../depend_stubs/quex/code_base/template/mode_handling.i
extract.o: ../depend_stubs/quex/code_base/template/misc.i
extract.o: ../depend_stubs/quex/code_base/template/buffer_access.i
extract.o: ../depend_stubs/quex/code_base/template/Analyser.i
extract.o: ../src/col_parser.h ../src/txdread.h
extract.o: ../../grit_core/src/physics/bcol_parser.h
extract.o: ../depend_stubs/stdint.h ../src/ios_util.h
extract.o: ../depend_stubs/sstream ../depend_stubs/cstring
extract.o: ../depend_stubs/cerrno ../../grit_core/src/portable_io.h
extract.o: ../depend_stubs/cassert
extract.o: ../../grit_core/src/centralised_log.h
extract.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
extract.o: ../../dependencies/util/console.h
extract.o: ../../dependencies/util/exception.h
extract.o: ../../dependencies/util/intrinsics.h ../src/dirutil.h
extract.o: ../src/csvread.h ../src/handling.h ../src/surfinfo.h
extract.o: ../src/procobj.h
handling.o: ../depend_stubs/cstdlib ../depend_stubs/iostream
handling.o: ../depend_stubs/fstream ../depend_stubs/algorithm
handling.o: ../depend_stubs/locale ../src/handling.h
handling.o: ../depend_stubs/istream ../depend_stubs/vector
handling.o: ../depend_stubs/string ../depend_stubs/map
handling.o: ../src/csvread.h ../src/ios_util.h ../depend_stubs/sstream
handling.o: ../depend_stubs/cstring ../depend_stubs/cerrno
handling.o: ../../grit_core/src/portable_io.h ../depend_stubs/cstdio
handling.o: ../depend_stubs/cassert ../depend_stubs/stdint.h
handling.o: ../../grit_core/src/centralised_log.h
handling.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
handling.o: ../../dependencies/util/console.h
handling.o: ../../dependencies/util/exception.h
handling.o: ../../dependencies/util/intrinsics.h
ideread.o: ../depend_stubs/climits ../depend_stubs/cmath
ideread.o: ../depend_stubs/cerrno ../depend_stubs/iostream
ideread.o: ../depend_stubs/string ../depend_stubs/sstream
ideread.o: ../depend_stubs/vector ../depend_stubs/fstream
ideread.o: ../depend_stubs/istream ../depend_stubs/iomanip
ideread.o: ../depend_stubs/algorithm ../depend_stubs/locale
ideread.o: ../src/ios_util.h ../depend_stubs/cstdlib
ideread.o: ../depend_stubs/cstring ../../grit_core/src/portable_io.h
ideread.o: ../depend_stubs/cstdio ../depend_stubs/cassert
ideread.o: ../depend_stubs/stdint.h
ideread.o: ../../grit_core/src/centralised_log.h
ideread.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
ideread.o: ../../dependencies/util/console.h
ideread.o: ../../dependencies/util/exception.h
ideread.o: ../../dependencies/util/intrinsics.h ../src/ideread.h
ideread.o: ../src/csvread.h ../depend_stubs/map
ifpread.o: ../depend_stubs/cstdlib ../depend_stubs/cmath
ifpread.o: ../depend_stubs/sstream ../depend_stubs/string
ifpread.o: ../depend_stubs/vector ../depend_stubs/iostream
ifpread.o: ../depend_stubs/fstream ../src/ios_util.h
ifpread.o: ../depend_stubs/cstring ../depend_stubs/cerrno
ifpread.o: ../../grit_core/src/portable_io.h ../depend_stubs/cstdio
ifpread.o: ../depend_stubs/cassert ../depend_stubs/stdint.h
ifpread.o: ../../grit_core/src/centralised_log.h
ifpread.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
ifpread.o: ../../dependencies/util/console.h
ifpread.o: ../../dependencies/util/exception.h
ifpread.o: ../../dependencies/util/intrinsics.h
imgread.o: ../depend_stubs/string ../depend_stubs/vector
imgread.o: ../depend_stubs/fstream ../depend_stubs/algorithm
imgread.o: ../depend_stubs/locale ../src/ios_util.h
imgread.o: ../depend_stubs/sstream ../depend_stubs/iostream
imgread.o: ../depend_stubs/cstdlib ../depend_stubs/cstring
imgread.o: ../depend_stubs/cerrno ../../grit_core/src/portable_io.h
imgread.o: ../depend_stubs/cstdio ../depend_stubs/cassert
imgread.o: ../depend_stubs/stdint.h
imgread.o: ../../grit_core/src/centralised_log.h
imgread.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
imgread.o: ../../dependencies/util/console.h
imgread.o: ../../dependencies/util/exception.h
imgread.o: ../../dependencies/util/intrinsics.h ../src/imgread.h
imgread.o: ../depend_stubs/map
iplread.o: ../depend_stubs/cmath ../depend_stubs/cerrno
iplread.o: ../depend_stubs/string ../depend_stubs/sstream
iplread.o: ../depend_stubs/vector ../depend_stubs/iostream
iplread.o: ../depend_stubs/fstream ../depend_stubs/algorithm
iplread.o: ../depend_stubs/locale ../src/ios_util.h
iplread.o: ../depend_stubs/cstdlib ../depend_stubs/cstring
iplread.o: ../../grit_core/src/portable_io.h ../depend_stubs/cstdio
iplread.o: ../depend_stubs/cassert ../depend_stubs/stdint.h
iplread.o: ../../grit_core/src/centralised_log.h
iplread.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
iplread.o: ../../dependencies/util/console.h
iplread.o: ../../dependencies/util/exception.h
iplread.o: ../../dependencies/util/intrinsics.h ../src/iplread.h
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
procobj.o: ../../grit_core/src/centralised_log.h
procobj.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
procobj.o: ../../dependencies/util/console.h
procobj.o: ../../dependencies/util/exception.h
procobj.o: ../../dependencies/util/intrinsics.h
surfinfo.o: ../depend_stubs/iostream ../depend_stubs/fstream
surfinfo.o: ../src/surfinfo.h ../depend_stubs/istream
surfinfo.o: ../depend_stubs/vector ../depend_stubs/string
surfinfo.o: ../depend_stubs/map ../src/csvread.h ../src/ios_util.h
surfinfo.o: ../depend_stubs/sstream ../depend_stubs/cstdlib
surfinfo.o: ../depend_stubs/cstring ../depend_stubs/cerrno
surfinfo.o: ../../grit_core/src/portable_io.h ../depend_stubs/cstdio
surfinfo.o: ../depend_stubs/cassert ../depend_stubs/stdint.h
surfinfo.o: ../../grit_core/src/centralised_log.h
surfinfo.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
surfinfo.o: ../../dependencies/util/console.h
surfinfo.o: ../../dependencies/util/exception.h
surfinfo.o: ../../dependencies/util/intrinsics.h
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
txdread.o: ../../grit_core/src/centralised_log.h
txdread.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
txdread.o: ../../dependencies/util/console.h
txdread.o: ../../dependencies/util/exception.h
txdread.o: ../../dependencies/util/intrinsics.h ../src/txdread.h
txdread.o: ../depend_stubs/set ../depend_stubs/istream
