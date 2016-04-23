
# TODO: move the engine stuff into dependencies/grit-util or grit-col or whatever.
# This should be easier once tcol is replaced with json.
EXTRACT_CPP_SRCS=\
	../engine/physics/bcol_parser.cpp \
	../engine/physics/tcol_parser.cpp \
	../engine/physics/tcol_lexer.cpp \
	../engine/physics/tcol_parser.cpp \
	../engine/physics/tcol_lexer-core-engine.cpp \
	../engine/physics/bcol_parser.cpp \
	col_parser.cpp \
	csvread.cpp \
	dffread.cpp \
	dirutil.cpp \
	extract.cpp \
	handling.cpp \
	ideread.cpp \
	imgread.cpp \
	iplread.cpp \
	procobj.cpp \
	surfinfo.cpp \
	tex_dups.cpp \
	txdread.cpp \

EXTRACT_INCLUDE_DIRS=\
    ../engine
