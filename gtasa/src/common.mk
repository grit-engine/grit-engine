# (c) David Cunningham 2011, Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php

COMPILING=echo "Compiling: [32m$<[0m"
LINKING=echo "Linking: [1;32m$@[0m"

%.o: ../src/%.cpp
	@$(COMPILING)
	@$(COMPILER) -c $< -o $@ $(CFLAGS)

LEXER=TColLexer
../src/$(LEXER): ../src/physics/$(LEXER).qx
	cd ../src && quex -i $< --engine $(LEXER) --token-queue 


imgread: ../src/imgread.cpp 
	@$(LINKING)
	@$(COMPILER) -D_IMGREAD_EXEC $^ -o $@ $(CFLAGS) $(CMDLINE_LDFLAGS)

ifpread: ../src/ifpread.cpp
	@$(LINKING)
	@$(COMPILER) -D_IFPREAD_EXEC $^ -o $@ $(CFLAGS) $(CMDLINE_LDFLAGS)

extract: ../src/extract.cpp ColParser.o BColParser.o TColParser.o ideread.o imgread.o dffread.o tex_dups.o iplread.o txdread.o dirutil.o csvread.o handling.o surfinfo.o procobj.o
	@$(LINKING)
	@$(COMPILER) $^ -o $@ $(CLDFLAGS)

ideread: ../src/ideread.cpp csvread.o
	@$(LINKING)
	@$(COMPILER) -D_IDEREAD_EXEC $^ -o $@ $(CFLAGS) $(CMDLINE_LDFLAGS)

dffread: ../src/dffread.cpp tex_dups.o ColParser.o TColParser.o
	@$(LINKING)
	@$(COMPILER) -D_DFFREAD_EXEC $^ -o $@ $(CFLAGS) $(CMDLINE_LDFLAGS)

txdread: ../src/txdread.cpp
	@$(LINKING)
	@$(COMPILER) -D_TXDREAD_EXEC $^ -o $@ $(CFLAGS) $(CMDLINE_LDFLAGS)

iplread: ../src/iplread.cpp csvread.o
	@$(LINKING)
	@$(COMPILER) -D_IPLREAD_EXEC $^ -o $@ $(CFLAGS) $(CMDLINE_LDFLAGS)

colread: colread.o ColParser.o TColParser.o $(LEXER).o $(LEXER)-core-engine.o 
	@$(LINKING)
	@$(COMPILER) $^ -o $@ $(CLDFLAGS)

csvread: ../src/csvread.cpp
	@$(LINKING)
	@$(COMPILER) -D_CSVREAD_EXEC $^ -o $@ $(CLDFLAGS)

handling: ../src/handling.cpp csvread.o
	@$(LINKING)
	@$(COMPILER) -D_HANDLING_EXEC $^ -o $@ $(CLDFLAGS)

surfinfo: ../src/surfinfo.cpp csvread.o
	@$(LINKING)
	@$(COMPILER) -D_SURFINFO_EXEC $^ -o $@ $(CLDFLAGS)

procobj: ../src/procobj.cpp csvread.o
	@$(LINKING)
	@$(COMPILER) -D_PROCOBJ_EXEC $^ -o $@ $(CLDFLAGS)


clean:
	rm -fv $(TARGETS) *.o

# DO NOT DELETE
