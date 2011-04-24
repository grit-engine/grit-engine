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
../src/$(LEXER): ../src/$(LEXER).qx
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

extract: ../src/extract.c++ ColParser.o TColParser.o ideread.o imgread.o dffread.o tex_dups.o iplread.o txdread.o dirutil.o csvread.o handling.o surfinfo.o procobj.o
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

ColParser.o: ../src/ColParser.h ../../grit_core/src/TColParser.h
ColParser.o: ../../grit_core/src/TColLexer
ColParser.o: ../../grit_core/src/TColLexer-token_ids
ColParser.o: ../../grit_core/src/math_util.h
ColParser.o: ../../grit_core/src/CentralisedLog.h
ColParser.o: ../../grit_core/src/console_colour.h ../src/ios_util.h
colread.o: ../src/ios_util.h ../../grit_core/src/TColParser.h
colread.o: ../../grit_core/src/TColLexer
colread.o: ../../grit_core/src/TColLexer-token_ids
colread.o: ../../grit_core/src/math_util.h
colread.o: ../../grit_core/src/CentralisedLog.h
colread.o: ../../grit_core/src/console_colour.h ../src/ColParser.h
csvread.o: ../src/csvread.h ../src/ios_util.h
dffread.o: ../src/ios_util.h ../src/dffread.h ../src/ideread.h
dffread.o: ../../grit_core/src/TColParser.h
dffread.o: ../../grit_core/src/TColLexer
dffread.o: ../../grit_core/src/TColLexer-token_ids
dffread.o: ../../grit_core/src/math_util.h
dffread.o: ../../grit_core/src/CentralisedLog.h
dffread.o: ../../grit_core/src/console_colour.h ../src/tex_dups.h
dffread.o: ../src/ColParser.h
dirutil.o: ../src/dirutil.h ../src/ios_util.h
extract.o: ../src/imgread.h ../src/iplread.h ../src/ideread.h
extract.o: ../src/tex_dups.h ../src/dffread.h
extract.o: ../../grit_core/src/TColParser.h
extract.o: ../../grit_core/src/TColLexer
extract.o: ../../grit_core/src/TColLexer-token_ids
extract.o: ../../grit_core/src/math_util.h
extract.o: ../../grit_core/src/CentralisedLog.h
extract.o: ../../grit_core/src/console_colour.h ../src/txdread.h
extract.o: ../src/ColParser.h ../src/ios_util.h ../src/dirutil.h
extract.o: ../src/csvread.h ../src/handling.h ../src/surfinfo.h
extract.o: ../src/procobj.h
handling.o: ../src/handling.h ../src/csvread.h ../src/ios_util.h
ideread.o: ../src/ideread.h ../src/ios_util.h ../src/csvread.h
ifpread.o: ../src/ios_util.h
imgread.o: ../src/ios_util.h ../src/imgread.h
iplread.o: ../src/iplread.h ../src/ios_util.h ../src/csvread.h
iplread.o: ../../grit_core/src/console_colour.h
procobj.o: ../src/procobj.h ../src/csvread.h ../src/ios_util.h
surfinfo.o: ../src/surfinfo.h ../src/csvread.h ../src/ios_util.h
tex_dups.o: ../src/tex_dups.h
txdread.o: ../src/ios_util.h ../src/txdread.h
