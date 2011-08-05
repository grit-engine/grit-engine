# (c) David Cunningham 2011, Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

COMPILING=echo "Compiling: [32m$<[0m"
LINKING=echo "Linking: [1;32m$@[0m"

%.o: ../src/win32/%.c++
	@$(COMPILING)
	@$(COMPILER) -c $< -o $@ $(CFLAGS)

%.o: ../src/linux/%.c++
	@$(COMPILING)
	@$(COMPILER) -c $< -o $@ $(CFLAGS)

%.o: ../src/%.c++
	@$(COMPILING)
	@$(COMPILER) -c $< -o $@ $(CFLAGS)

LEXER=TColLexer
../src/$(LEXER): ../src/physics/$(LEXER).qx
	cd ../src && quex -i $< --engine $(LEXER) --token-queue 

%.o: ../src/%.cpp
	@$(COMPILING)
	@$(WEAKCOMPILER) -c $< -o $@ $(CFLAGS)


imgread: ../src/imgread.c++ 
	@$(LINKING)
	@$(COMPILER) -D_IMGREAD_EXEC $^ -o $@ $(CFLAGS) $(CMDLINE_LDFLAGS)

ifpread: ../src/ifpread.c++
	@$(LINKING)
	@$(COMPILER) -D_IFPREAD_EXEC $^ -o $@ $(CFLAGS) $(CMDLINE_LDFLAGS)

extract: ../src/extract.c++ ColParser.o BColParser.o TColParser.o ideread.o imgread.o dffread.o tex_dups.o iplread.o txdread.o dirutil.o csvread.o handling.o surfinfo.o procobj.o
	@$(LINKING)
	@$(COMPILER) $^ -o $@ $(CLDFLAGS)

ideread: ../src/ideread.c++ csvread.o
	@$(LINKING)
	@$(COMPILER) -D_IDEREAD_EXEC $^ -o $@ $(CFLAGS) $(CMDLINE_LDFLAGS)

dffread: ../src/dffread.c++ tex_dups.o ColParser.o TColParser.o
	@$(LINKING)
	@$(COMPILER) -D_DFFREAD_EXEC $^ -o $@ $(CFLAGS) $(CMDLINE_LDFLAGS)

txdread: ../src/txdread.c++
	@$(LINKING)
	@$(COMPILER) -D_TXDREAD_EXEC $^ -o $@ $(CFLAGS) $(CMDLINE_LDFLAGS)

iplread: ../src/iplread.c++ csvread.o
	@$(LINKING)
	@$(COMPILER) -D_IPLREAD_EXEC $^ -o $@ $(CFLAGS) $(CMDLINE_LDFLAGS)

colread: colread.o ColParser.o TColParser.o $(LEXER).o $(LEXER)-core-engine.o 
	@$(LINKING)
	@$(COMPILER) $^ -o $@ $(CLDFLAGS)

csvread: ../src/csvread.c++
	@$(LINKING)
	@$(COMPILER) -D_CSVREAD_EXEC $^ -o $@ $(CLDFLAGS)

handling: ../src/handling.c++ csvread.o
	@$(LINKING)
	@$(COMPILER) -D_HANDLING_EXEC $^ -o $@ $(CLDFLAGS)

surfinfo: ../src/surfinfo.c++ csvread.o
	@$(LINKING)
	@$(COMPILER) -D_SURFINFO_EXEC $^ -o $@ $(CLDFLAGS)

procobj: ../src/procobj.c++ csvread.o
	@$(LINKING)
	@$(COMPILER) -D_PROCOBJ_EXEC $^ -o $@ $(CLDFLAGS)


clean:
	rm -fv $(TARGETS) *.o

# DO NOT DELETE

ColParser.o: ../depend_stubs/locale ../depend_stubs/algorithm
ColParser.o: ../depend_stubs/iostream ../depend_stubs/fstream
ColParser.o: ../src/ColParser.h
ColParser.o: ../../grit_core/src/physics/TColParser.h
ColParser.o: ../depend_stubs/string ../../grit_core/src/math_util.h
ColParser.o: ../depend_stubs/cmath ../depend_stubs/cfloat
ColParser.o: ../../grit_core/src/CentralisedLog.h
ColParser.o: ../depend_stubs/sstream
ColParser.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
ColParser.o: ../../grit_core/src/console_colour.h
ColParser.o: ../../grit_core/src/physics/TColLexer
ColParser.o: ../depend_stubs/vector ../depend_stubs/map
ColParser.o: ../depend_stubs/istream
ColParser.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
ColParser.o: ../../grit_core/src/physics/TColLexer-token_ids
ColParser.o: ../depend_stubs/cstdio
ColParser.o: ../depend_stubs/quex/code_base/Token
ColParser.o: ../depend_stubs/quex/code_base/definitions
ColParser.o: ../depend_stubs/quex/code_base/buffer/Buffer
ColParser.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
ColParser.o: ../depend_stubs/quex/code_base/template/QuexMode
ColParser.o: ../depend_stubs/quex/code_base/template/Analyser
ColParser.o: ../depend_stubs/quex/code_base/template/Counter
ColParser.o: ../depend_stubs/quex/code_base/template/Accumulator
ColParser.o: ../depend_stubs/quex/code_base/TokenQueue
ColParser.o: ../depend_stubs/quex/code_base/template/IncludeStack
ColParser.o: ../depend_stubs/cstdlib
ColParser.o: ../depend_stubs/quex/code_base/temporary_macros_on
ColParser.o: ../depend_stubs/quex/code_base/temporary_macros_off
ColParser.o: ../depend_stubs/quex/code_base/template/constructor.i
ColParser.o: ../depend_stubs/quex/code_base/template/Counter.i
ColParser.o: ../depend_stubs/quex/code_base/template/Accumulator.i
ColParser.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
ColParser.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
ColParser.o: ../depend_stubs/quex/code_base/template/mode_handling.i
ColParser.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
ColParser.o: ../depend_stubs/quex/code_base/template/misc.i
ColParser.o: ../depend_stubs/quex/code_base/template/buffer_access.i
ColParser.o: ../depend_stubs/quex/code_base/template/Analyser.i
ColParser.o: ../src/ios_util.h ../depend_stubs/cstring
ColParser.o: ../depend_stubs/cerrno ../../grit_core/src/portable_io.h
ColParser.o: ../depend_stubs/cassert ../depend_stubs/stdint.h
colread.o: ../depend_stubs/cstdlib ../depend_stubs/cstring
colread.o: ../depend_stubs/cerrno ../depend_stubs/iostream
colread.o: ../depend_stubs/fstream ../src/ios_util.h
colread.o: ../depend_stubs/sstream ../../grit_core/src/portable_io.h
colread.o: ../depend_stubs/cstdio ../depend_stubs/cassert
colread.o: ../depend_stubs/string ../depend_stubs/stdint.h
colread.o: ../../grit_core/src/CentralisedLog.h
colread.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
colread.o: ../../grit_core/src/console_colour.h
colread.o: ../../grit_core/src/physics/TColParser.h
colread.o: ../../grit_core/src/math_util.h ../depend_stubs/cmath
colread.o: ../depend_stubs/cfloat
colread.o: ../../grit_core/src/physics/TColLexer
colread.o: ../depend_stubs/vector ../depend_stubs/map
colread.o: ../depend_stubs/istream
colread.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
colread.o: ../../grit_core/src/physics/TColLexer-token_ids
colread.o: ../depend_stubs/quex/code_base/Token
colread.o: ../depend_stubs/quex/code_base/definitions
colread.o: ../depend_stubs/quex/code_base/buffer/Buffer
colread.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
colread.o: ../depend_stubs/quex/code_base/template/QuexMode
colread.o: ../depend_stubs/quex/code_base/template/Analyser
colread.o: ../depend_stubs/quex/code_base/template/Counter
colread.o: ../depend_stubs/quex/code_base/template/Accumulator
colread.o: ../depend_stubs/quex/code_base/TokenQueue
colread.o: ../depend_stubs/quex/code_base/template/IncludeStack
colread.o: ../depend_stubs/quex/code_base/temporary_macros_on
colread.o: ../depend_stubs/quex/code_base/temporary_macros_off
colread.o: ../depend_stubs/quex/code_base/template/constructor.i
colread.o: ../depend_stubs/quex/code_base/template/Counter.i
colread.o: ../depend_stubs/quex/code_base/template/Accumulator.i
colread.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
colread.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
colread.o: ../depend_stubs/quex/code_base/template/mode_handling.i
colread.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
colread.o: ../depend_stubs/quex/code_base/template/misc.i
colread.o: ../depend_stubs/quex/code_base/template/buffer_access.i
colread.o: ../depend_stubs/quex/code_base/template/Analyser.i
colread.o: ../src/ColParser.h
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
csvread.o: ../../grit_core/src/console_colour.h
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
dffread.o: ../../grit_core/src/console_colour.h ../src/dffread.h
dffread.o: ../depend_stubs/set ../src/ideread.h
dffread.o: ../depend_stubs/istream
dffread.o: ../../grit_core/src/physics/TColParser.h
dffread.o: ../../grit_core/src/math_util.h ../depend_stubs/cmath
dffread.o: ../depend_stubs/cfloat
dffread.o: ../../grit_core/src/physics/TColLexer
dffread.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
dffread.o: ../../grit_core/src/physics/TColLexer-token_ids
dffread.o: ../depend_stubs/quex/code_base/Token
dffread.o: ../depend_stubs/quex/code_base/definitions
dffread.o: ../depend_stubs/quex/code_base/buffer/Buffer
dffread.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
dffread.o: ../depend_stubs/quex/code_base/template/QuexMode
dffread.o: ../depend_stubs/quex/code_base/template/Analyser
dffread.o: ../depend_stubs/quex/code_base/template/Counter
dffread.o: ../depend_stubs/quex/code_base/template/Accumulator
dffread.o: ../depend_stubs/quex/code_base/TokenQueue
dffread.o: ../depend_stubs/quex/code_base/template/IncludeStack
dffread.o: ../depend_stubs/quex/code_base/temporary_macros_on
dffread.o: ../depend_stubs/quex/code_base/temporary_macros_off
dffread.o: ../depend_stubs/quex/code_base/template/constructor.i
dffread.o: ../depend_stubs/quex/code_base/template/Counter.i
dffread.o: ../depend_stubs/quex/code_base/template/Accumulator.i
dffread.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
dffread.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
dffread.o: ../depend_stubs/quex/code_base/template/mode_handling.i
dffread.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
dffread.o: ../depend_stubs/quex/code_base/template/misc.i
dffread.o: ../depend_stubs/quex/code_base/template/buffer_access.i
dffread.o: ../depend_stubs/quex/code_base/template/Analyser.i
dffread.o: ../src/ColParser.h ../src/tex_dups.h
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
dirutil.o: ../../grit_core/src/console_colour.h
extract.o: ../depend_stubs/cstdlib ../depend_stubs/cmath
extract.o: ../depend_stubs/iostream ../depend_stubs/fstream
extract.o: ../depend_stubs/locale ../depend_stubs/algorithm
extract.o: ../src/imgread.h ../depend_stubs/string
extract.o: ../depend_stubs/vector ../depend_stubs/map ../src/iplread.h
extract.o: ../src/ideread.h ../depend_stubs/istream ../src/tex_dups.h
extract.o: ../src/dffread.h ../depend_stubs/set
extract.o: ../../grit_core/src/physics/TColParser.h
extract.o: ../../grit_core/src/math_util.h ../depend_stubs/cfloat
extract.o: ../../grit_core/src/CentralisedLog.h
extract.o: ../depend_stubs/sstream
extract.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
extract.o: ../../grit_core/src/console_colour.h
extract.o: ../../grit_core/src/physics/TColLexer
extract.o: ../depend_stubs/quex/code_base/compatibility/inttypes.h
extract.o: ../../grit_core/src/physics/TColLexer-token_ids
extract.o: ../depend_stubs/cstdio ../depend_stubs/quex/code_base/Token
extract.o: ../depend_stubs/quex/code_base/definitions
extract.o: ../depend_stubs/quex/code_base/buffer/Buffer
extract.o: ../depend_stubs/quex/code_base/buffer/plain/BufferFiller_Plain
extract.o: ../depend_stubs/quex/code_base/template/QuexMode
extract.o: ../depend_stubs/quex/code_base/template/Analyser
extract.o: ../depend_stubs/quex/code_base/template/Counter
extract.o: ../depend_stubs/quex/code_base/template/Accumulator
extract.o: ../depend_stubs/quex/code_base/TokenQueue
extract.o: ../depend_stubs/quex/code_base/template/IncludeStack
extract.o: ../depend_stubs/quex/code_base/temporary_macros_on
extract.o: ../depend_stubs/quex/code_base/temporary_macros_off
extract.o: ../depend_stubs/quex/code_base/template/constructor.i
extract.o: ../depend_stubs/quex/code_base/template/Counter.i
extract.o: ../depend_stubs/quex/code_base/template/Accumulator.i
extract.o: ../depend_stubs/quex/code_base/template/token_receiving_via_queue.i
extract.o: ../depend_stubs/quex/code_base/template/token_sending_via_queue.i
extract.o: ../depend_stubs/quex/code_base/template/mode_handling.i
extract.o: ../depend_stubs/quex/code_base/template/IncludeStack.i
extract.o: ../depend_stubs/quex/code_base/template/misc.i
extract.o: ../depend_stubs/quex/code_base/template/buffer_access.i
extract.o: ../depend_stubs/quex/code_base/template/Analyser.i
extract.o: ../src/ColParser.h ../src/txdread.h
extract.o: ../../grit_core/src/physics/BColParser.h
extract.o: ../depend_stubs/stdint.h ../src/ios_util.h
extract.o: ../depend_stubs/cstring ../depend_stubs/cerrno
extract.o: ../../grit_core/src/portable_io.h ../depend_stubs/cassert
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
handling.o: ../../grit_core/src/console_colour.h
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
ideread.o: ../../grit_core/src/console_colour.h ../src/csvread.h
ideread.o: ../depend_stubs/map
ifpread.o: ../depend_stubs/cstdlib ../depend_stubs/cmath
ifpread.o: ../depend_stubs/sstream ../depend_stubs/string
ifpread.o: ../depend_stubs/vector ../depend_stubs/iostream
ifpread.o: ../depend_stubs/fstream ../src/ios_util.h
ifpread.o: ../depend_stubs/cstring ../depend_stubs/cerrno
ifpread.o: ../../grit_core/src/portable_io.h ../depend_stubs/cstdio
ifpread.o: ../depend_stubs/cassert ../depend_stubs/stdint.h
ifpread.o: ../../grit_core/src/CentralisedLog.h
ifpread.o: ../depend_stubs/boost/thread/recursive_mutex.hpp
ifpread.o: ../../grit_core/src/console_colour.h
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
imgread.o: ../../grit_core/src/console_colour.h ../src/imgread.h
imgread.o: ../depend_stubs/map
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
iplread.o: ../../grit_core/src/console_colour.h ../src/csvread.h
iplread.o: ../depend_stubs/istream ../depend_stubs/map
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
procobj.o: ../../grit_core/src/console_colour.h
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
surfinfo.o: ../../grit_core/src/console_colour.h
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
txdread.o: ../../grit_core/src/console_colour.h ../src/txdread.h
txdread.o: ../depend_stubs/set ../depend_stubs/istream
