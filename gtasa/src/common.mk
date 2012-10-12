# (c) David Cunningham 2011, Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

COMPILING=echo "Compiling: [32m$<[0m"
LINKING=echo "Linking: [1;32m$@[0m"

ALL_OPTS=$(CXXFLAGS) $(LDFLAGS) $(LDLIBS)

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
	@$(CXX) -pedantic $^ -o $@ $(CLDFLAGS)

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

colread: colread.o ColParser.o TColParser.o TColLexer.o $TColLexercore-engine.o 
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

# DO NOT DELETE
