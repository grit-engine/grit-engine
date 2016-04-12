# PURPOSE: Makefile Demo Application of Quex
#
# ABSOLUTELY NO WARRANTY
#_______________________________________________________________________________
.PHONY: clean

ifndef QUEX_PATH
    $(error The environment variable QUEX_PATH is not defined!)
endif

include $(QUEX_PATH)/quex/code_base/core.mkd

# (*) SETUP ____________________________________________________________________
SOURCES  = tiny_lexer  tiny_lexer.cpp  tiny_lexer-token_ids  tiny_lexer-core-engine.cpp
WSOURCES = tiny_wlexer tiny_wlexer.cpp tiny_wlexer-token_ids tiny_wlexer-core-engine.cpp 
# (*) COMPILER SETTINGS ________________________________________________________
#     (change COMPILER to whatever you use as compiler on the command line,
#      e.g. "make COMPILER=icpc" will use intel's c++ compiler)
COMPILER = g++ -ggdb -Wall -pedantic -DQUEX_OPTION_NO_COMPUTED_GOTOS
CC = $(COMPILER) -c -I./ -I$(QUEX_PATH) $(NDEBUG_F) -DQUEX_OPTION_ASSERTS
LD = $(COMPILER) 

# (*) RULES ____________________________________________________________________
all: lexer stdinlexer wlexer

# -- char application
lexer: lexer.o tiny_lexer.o tiny_lexer-core-engine.o
	$(LD) -o $@  lexer.o tiny_lexer.o tiny_lexer-core-engine.o 
          
stdinlexer: stdinlexer.o tiny_lexer.o tiny_lexer-core-engine.o
	$(LD) -o $@  stdinlexer.o tiny_lexer.o tiny_lexer-core-engine.o 
          
$(SOURCES): simple.qx $(QUEX_CORE)
	quex -i simple.qx --engine tiny_lexer # CERTAINLY NOT: "-b wchar_t" 

# -- wchar_t application
wlexer: wlexer.o tiny_wlexer.o tiny_wlexer-core-engine.o
	$(LD) -o $@  wlexer.o tiny_wlexer.o tiny_wlexer-core-engine.o 
          
$(WSOURCES): simple.qx $(QUEX_CORE)
	quex -i simple.qx --engine tiny_wlexer -b wchar_t 

# -- The general way to get .o from .cpp
%.o: %.cpp $(SOURCES) $(WSOURCES)
	$(CC) $< -o $@ 

# (*) HELPERS __________________________________________________________________
clean:	
	touch simple.qx
	rm -f $(SOURCES)
	rm -f $(WSOURCES)
	rm -f *.o
	rm -f lexer stdinlexer wlexer
	rm -f *.bak
