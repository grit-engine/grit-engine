# PURPOSE: Makefile Demo Application of Quex
#
# ABSOLUTELY NO WARRANTY
#_______________________________________________________________________________
.PHONY: clean

ifndef QUEX_PATH
    $(error The environment variable QUEX_PATH is not defined!)
endif

# (*) SETUP ____________________________________________________________________
# -- INPUT
LEXER_FILE		= Calc_lexer.qx

# -- FILES PRODUCED BY QUEX
LEXER_NAME		= Calc_lexer# NOTE: a whitespace after this name creates chaos!
LEXER_SOURCES	= $(LEXER_NAME)                     \
                  $(LEXER_NAME).cpp                 \
                  $(LEXER_NAME)-token_ids           \
		          $(LEXER_NAME)-core-engine.cpp

TOKEN_FILE		= Calc_token-ids

PARSER_NAME		=	Calc_parser
PARSER_FILE		=	$(PARSER_NAME).ypp
PARSER_SOURCES	=	$(PARSER_NAME).tab.cpp	\
					$(PARSER_NAME).tab.hpp
# -- OUTPUT
APPLICATION      = lexer

# (*) COMPILER SETTINGS ________________________________________________________
#     (change COMPILER to whatever you use as compiler on the command line,
#      e.g. "make COMPILER=icpc" will use intel's c++ compiler)
COMPILER = g++ -ggdb -Wall

ifneq (,$(findstring $(COMPILER),icpc cl dmc))
	LIBICONV=-liconv
else
	LIBICONV=#
endif

CC = $(COMPILER) -c -I./ -I$(QUEX_PATH) $(NDEBUG_F) \
       -DQUEX_OPTION_ACTIVATE_ASSERTS # -no-deprecated -Wall -fPIC

LD = $(COMPILER) $(LIBICONV)

# (*) RULES ____________________________________________________________________
# -- application
$(APPLICATION): main.o $(LEXER_NAME).o $(LEXER_NAME)-core-engine.o $(PARSER_NAME).tab.o
	$(LD) -o $(APPLICATION) main.o $(LEXER_NAME).o $(LEXER_NAME)-core-engine.o $(PARSER_NAME).tab.o

# -- engine and object files
%.o: %.cpp $(LEXER_SOURCES) $(PARSER_NAME).tab.cpp
	$(CC) $< -o $@ # -D__QUEX_OPTION_DEBUG_STATE_TRANSITION_REPORTS

$(LEXER_SOURCES): $(LEXER_FILE) $(PARSER_NAME).tab.cpp
	quex -i $(LEXER_FILE) --engine $(LEXER_NAME) --token-prefix TKN_ \
          --foreign-token-id-file Calc_token-ids.h

$(PARSER_NAME).tab.cpp: $(PARSER_FILE)
	bison $(PARSER_FILE)

# (*) HELPERS __________________________________________________________________
clean:	
	touch $(LEXER_FILE)
	rm -f $(LEXER_SOURCES)
	rm -f $(PARSER_SOURCES)
	rm -f *.o
	rm -f $(APPLICATION)
	rm -f *.bak
