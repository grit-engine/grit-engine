# PURPOSE: Makefile for Demo Application of Quex
#
# ABSOLUTELY NO WARRANTY
#_______________________________________________________________________________
.PHONY: clean

ifndef QUEX_PATH
    $(error The environment variable QUEX_PATH is not defined!)
endif

ifndef BYTES_PER_CHARACTER
    BYTES_PER_CHARACTER = 2
endif
include $(QUEX_PATH)/quex/code_base/core.mkd

# (*) SETUP ____________________________________________________________________
# -- INPUT
MODE_FILES       = definitions.qx end_of_file.qx program.qx
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
#     e.g. "make COMPILER=icpc" will use intel's c++ compiler)
COMPILER = g++
# Some compiler distributions have iconv in a separate lib:
#    -- Intel's icpc
#    -- MS's cl
#    -- Digital Mars dmc
# Add any compiler that does so to
ifneq (,$(findstring $(COMPILER),icpc cl dmc))
	LIBICONV=-liconv
else
	LIBICONV=#
endif
CC = $(COMPILER) -c -fPIC -Wno-deprecated -Wall \
	 -I./ -I$(QUEX_PATH) $(NDEBUG_F) \
	 -D___QUEX_UNIT_TESTING___ -ggdb \
     -DQUEX_OPTION_ACTIVATE_ASSERTS	

LD = $(COMPILER) $(LIBICONV) 

# (*) RULES ____________________________________________________________________
# -- application
$(APPLICATION): $(APPLICATION).o \
	            $(ENGINE_NAME).o $(ENGINE_NAME)-core-engine.o
	$(LD) ./$(APPLICATION).o $(ENGINE_NAME).o $(ENGINE_NAME)-core-engine.o \
          -o $(APPLICATION) 
# -- libraries
$(LIBRARY_NAME).a:
	$(STATIC_LD) $@ $(LIBRARY_OBJS)

$(LIBRARY_NAME).so:
	$(DYNAMIC_LD) $(LIBRARY_OBJS) -o $@ 

# -- engine and object files
%.o: %.cpp $(ENGINE_SOURCES)
	$(CC) $< -o $@

$(ENGINE_SOURCES): $(MODE_FILES) $(QUEX_CORE)
	quex -i $(MODE_FILES) --engine $(ENGINE_NAME)          \
	     --bytes-per-ucs-code-point $(BYTES_PER_CHARACTER) \
	     --iconv 

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
