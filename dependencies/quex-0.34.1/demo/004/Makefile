# PURPOSE: Makefile Demo Application of Quex
#
# ABSOLUTELY NO WARRANTY
#_______________________________________________________________________________
COMPILER     = g++    # icpc # g++ 
COMPILER_V   = 4.3.1  # 10.1 # 4.3.1
BUFFER_SIZE  = 65536
OPTIMIZATION = -O3 # -ggdb # -O3    # --coverage #-ggdb # -O1 #-pg # -O3# -pg -Os

# (*) SETUP ____________________________________________________________________
SETUP=-D'EMAIL="fschaef@users.sourceforge.net"' \
      -D'CPU_NAME="IntelCoreDuo"' \
      -D'CPU_CODE="T2400"' \
      -D'CPU_FREQ_MHZ=((float)(1833.0))' \
	  -D'CC_OPTIMIZATION_FLAGS="$(OPTIMIZATION)"' \
      -D'CHARACTER_SIZE=((unsigned)(1))' \
	  -D'OS_NAME="linux-2"' \
	  -D'CC_NAME="$(COMPILER)"' \
	  -D'CC_VERSION="$(COMPILER_V)"' \
      -D'QUEX_SETTING_BUFFER_SIZE=$(BUFFER_SIZE)' \
	  -D'NOTE=""'

.PHONY: clean

ifndef QUEX_PATH
    $(error The environment variable QUEX_PATH is not defined!)
endif
include $(QUEX_PATH)/quex/code_base/core.mkd

# -- INPUT
MODE_FILES       = c.qx
# -- FILES PRODUCED BY QUEX
ENGINE_NAME      = c_lexer# NOTE: a whitespace after this name creates chaos!
ENGINE_SOURCES   = $(ENGINE_NAME)                     \
                   $(ENGINE_NAME).cpp                 \
                   $(ENGINE_NAME)-token_ids           \
		           $(ENGINE_NAME)-core-engine.cpp
APPLICATION  = lexer
SOURCE_FILES = lexer.cpp report.cpp \
			   $(ENGINE_NAME).cpp $(ENGINE_NAME)-core-engine.cpp

OBJECT_FILES    = $(SOURCE_FILES:.cpp=.o)    # object file w/o line and column counting
OBJECT_FILES_LC = $(OBJECT_FILES:.o=-lc.o) # object file w line and column counting

# (*) COMPILER SETTINGS ________________________________________________________
#     (change COMPILER to whatever you use as compiler on the command line,
#     e.g. "make COMPILER=icpc" will use intel's c++ compiler)
PROFILE_ACTIVATE = -fprofile-arcs -fprofile-values
PROFILE_USE      = -fbranch-probabilities
CC = $(COMPILER) $(OPTIMIZATION) -Wno-deprecated -Wall \
	 -I./ -I$(QUEX_PATH)  \
	 $(SETUP) \
     -D__QUEX_CORE_OPTION_SUPPORT_BEGIN_OF_LINE_PRE_CONDITION_DISABLED \
	 -DNDEBUG \
     -DQUEX_OPTION_ASSERTS_DISABLED	\
	 -DQUEX_OPTION_AUTOMATIC_ANALYSIS_CONTINUATION_ON_MODE_CHANGE_DISABLED \
	 -DQUEX_BENCHMARK_SERIOUS \
     #-DQUEX_SETTING_BUFFER_SIZE=64 \
	 # -D__QUEX_OPTION_UNIT_TEST \
     # -D__QUEX_OPTION_DEBUG_STATE_TRANSITION_REPORTS \
     # -D__QUEX_OPTION_UNIT_TEST_QUEX_BUFFER_LOADS \

LD = $(COMPILER) $(OPTIMIZATION) 

# (*) RULES ____________________________________________________________________
#    lexer    --> application w/o line or column counting
#    lexer-lc --> application w/ line and column counting
all: lexer lexer-lc

lexer: $(OBJECT_FILES)
	echo $(OBJECT_FILES)
	$(LD) $(OBJECT_FILES)  -o $@ # -lgcov

lexer-lc: $(OBJECT_FILES_LC)
	$(LD) $(OBJECT_FILES_LC) -o $@ # -lgcov

# -- application
%.o: %.cpp c_lexer
	$(CC) -c \
        -DQUEX_OPTION_LINE_NUMBER_COUNTING_DISABLED \
        -DQUEX_OPTION_COLUMN_NUMBER_COUNTING_DISABLED \
		$< -o $@

%-lc.o: %.cpp c_lexer
	$(CC) -c \
        -DQUEX_OPTION_LINE_NUMBER_COUNTING   \
        -DQUEX_OPTION_COLUMN_NUMBER_COUNTING \
		$< -o $@

%.asm: %.cpp c_lexer
	$(CC) -S \
        -DQUEX_OPTION_LINE_NUMBER_COUNTING_DISABLED \
        -DQUEX_OPTION_COLUMN_NUMBER_COUNTING_DISABLED \
		$< -o $@

$(ENGINE_SOURCES): $(MODE_FILES) $(QUEX_CORE)
	quex -i $(MODE_FILES) --engine $(ENGINE_NAME) --token-offset 3 --no-token-queue --no-string-accumulator

# (*) HELPERS __________________________________________________________________
clean:	
	rm -f lexer lexer-lc
	touch $(MODE_FILES)
	rm -f $(ENGINE_SOURCES)
	rm -f $(ENGINE_NAME).o
	rm -f $(ENGINE_NAME)-core-engine.o
	rm -f lexer.o
	rm -f report.o
	rm -f lexer
	rm -f token_ids
	rm -f *.bak
	rm -f *.asm
	rm -f *.gcno 
	rm -f *.gcda
	rm -f *.gcov

