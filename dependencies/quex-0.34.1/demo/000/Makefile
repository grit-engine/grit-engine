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
# -- INPUT
MODE_FILES       = simple.qx
# -- FILES PRODUCED BY QUEX
ENGINE_NAME      = tiny_lexer# NOTE: a whitespace after this name creates chaos!
ENGINE_SOURCES   = $(ENGINE_NAME)                     \
                   $(ENGINE_NAME).cpp                 \
                   $(ENGINE_NAME)-token_ids           \
		           $(ENGINE_NAME)-core-engine.cpp
# -- OUTPUT
APPLICATION      = lexer

# (*) COMPILER SETTINGS ________________________________________________________
#     (change COMPILER to whatever you use as compiler on the command line,
#      e.g. "make COMPILER=icpc" will use intel's c++ compiler)
COMPILER = g++ -ggdb -Wall -pedantic -DQUEX_OPTION_NO_COMPUTED_GOTOS
CC = $(COMPILER) -c \
	 -I./ -I$(QUEX_PATH) $(NDEBUG_F) \
     -DQUEX_OPTION_ASSERTS	
	 # -no-deprecated -Wall \
	 #  -fPIC \

LD = $(COMPILER) 

# (*) RULES ____________________________________________________________________
# -- application
$(APPLICATION): $(APPLICATION).o \
	            $(ENGINE_NAME).o $(ENGINE_NAME)-core-engine.o
	$(LD) -o $(APPLICATION) \
		  ./$(APPLICATION).o $(ENGINE_NAME).o $(ENGINE_NAME)-core-engine.o 
          

# -- engine and object files
%.o: %.cpp $(ENGINE_SOURCES)
	$(CC) $< -o $@ # -D__QUEX_OPTION_DEBUG_STATE_TRANSITION_REPORTS

$(ENGINE_SOURCES): $(MODE_FILES) $(QUEX_CORE)
	quex -i $(MODE_FILES) --engine $(ENGINE_NAME) \
		 --token-offset 0x1000 \

# (*) HELPERS __________________________________________________________________
clean:	
	touch $(MODE_FILES)
	rm -f $(ENGINE_SOURCES)
	rm -f $(ENGINE_NAME).o
	rm -f $(ENGINE_NAME)-core-engine.o
	rm -f lexer.o
	rm -f lexer
	rm -f token_ids
	rm -f *.bak
