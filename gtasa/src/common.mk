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
	@$(COMPILER) -D_IMGREAD_TEST $^ -o $@ $(CFLAGS) $(CMDLINE_LDFLAGS)

ifpread: ../src/ifpread.c++
	@$(LINKING)
	@$(COMPILER) -D_IFPREAD_TEST $^ -o $@ $(CFLAGS) $(CMDLINE_LDFLAGS)

extract: ../src/extract.c++ ColParser.o TColParser.o ideread.o imgread.o dffread.o tex_dups.o iplread.o txdread.o dirutil.o
	@$(LINKING)
	@$(COMPILER) $^ -o $@ $(CLDFLAGS)

ideread: ../src/ideread.c++
	@$(LINKING)
	@$(COMPILER) -D_IDEREAD_TEST $^ -o $@ $(CFLAGS) $(CMDLINE_LDFLAGS)

dffread: ../src/dffread.c++ tex_dups.o 
	@$(LINKING)
	@$(COMPILER) -D_DFFREAD_TEST $^ -o $@ $(CFLAGS) $(CMDLINE_LDFLAGS)

iplread: ../src/iplread.c++
	@$(LINKING)
	@$(COMPILER) -D_IPLREAD_TEST $^ -o $@ $(CFLAGS) $(CMDLINE_LDFLAGS)

colread: colread.o ColParser.o TColParser.o $(LEXER).o $(LEXER)-core-engine.o 
	@$(LINKING)
	@$(COMPILER) $^ -o $@ $(CLDFLAGS)


clean:
	rm -fv $(TARGETS) *.o

# DO NOT DELETE

ColParser.o: ../src/ColParser.h ../../grit_core/src/TColParser.h
ColParser.o: ../../grit_core/src/TColLexer
ColParser.o: ../../grit_core/src/TColLexer-token_ids ../src/ios_util.h
colread.o: ../src/ios_util.h ../../grit_core/src/TColParser.h
colread.o: ../../grit_core/src/TColLexer
colread.o: ../../grit_core/src/TColLexer-token_ids ../src/ColParser.h
dffread.o: ../src/ios_util.h ../src/dffread.h ../src/ideread.h
dffread.o: ../src/tex_dups.h
dirutil.o: ../src/dirutil.h ../src/ios_util.h
extract.o: ../src/imgread.h ../src/iplread.h ../src/ideread.h
extract.o: ../src/tex_dups.h ../src/dffread.h ../src/txdread.h
extract.o: ../../grit_core/src/TColParser.h
extract.o: ../../grit_core/src/TColLexer
extract.o: ../../grit_core/src/TColLexer-token_ids ../src/ColParser.h
extract.o: ../src/ios_util.h ../src/dirutil.h
ideread.o: ../src/ideread.h ../src/ios_util.h
ifpread.o: ../src/ios_util.h
imgread.o: ../src/ios_util.h ../src/imgread.h
iplread.o: ../src/iplread.h ../src/ios_util.h
iplread.o: ../../grit_core/src/console_colour.h
tex_dups.o: ../src/tex_dups.h
txdread.o: ../src/ios_util.h ../src/txdread.h
