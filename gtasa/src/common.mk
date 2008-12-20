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
$(LEXER): ../src/$(LEXER).qx
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

iplread: ../src/iplread.c++
	@$(LINKING)
	@$(COMPILER) -D_IPLREAD_TEST $^ -o $@ $(CFLAGS) $(CMDLINE_LDFLAGS)

colread: colread.o ColParser.o TColParser.o $(LEXER).o $(LEXER)-core-engine.o 
	@$(LINKING)
	@$(COMPILER) $^ -o $@ $(CLDFLAGS)


clean:
	rm -fv $(TARGETS) *.o

