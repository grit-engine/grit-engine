#! /usr/bin/env python
import os
from copy import copy
import time

from quex.frs_py.string_handling import blue_print
from quex.frs_py.file_in  import open_file_or_die, \
                                 get_include_guard_extension

import quex.lexer_mode              as lexer_mode
import quex.output.cpp.mode_classes as mode_classes


def do(Modes, Setup):

    write_engine_header(Modes, Setup)

    write_mode_class_implementation(Modes, Setup)

def write_engine_header(Modes, Setup):

    QuexClassHeaderFileTemplate = (Setup.QUEX_TEMPLATE_DB_DIR 
                                   + "/template/lexical_analyzer_class").replace("//","/")
    CoreEngineDefinitionsHeader = (Setup.QUEX_TEMPLATE_DB_DIR + "/core_engine/").replace("//","/")
    QuexClassHeaderFileOutput   = Setup.output_file_stem
    LexerClassName              = Setup.output_engine_name
    VersionID                   = Setup.input_application_version_id
    QuexVersionID               = Setup.QUEX_VERSION

    # -- determine character type according to number of bytes per ucs character code point
    #    for the internal engine.
    quex_character_type_str = { 1: "uint8_t ", 2: "uint16_t", 4: "uint32_t", 
                                   "wchar_t": "wchar_t" }[Setup.bytes_per_ucs_code_point]
    quex_lexeme_type_str    = { 1: "char    ", 2: "int16_t",  4: "int32_t",  
                                   "wchar_t": "wchar_t" }[Setup.bytes_per_ucs_code_point]

    #    are bytes of integers Setup 'little endian' or 'big endian' ?
    if Setup.byte_order == "little":
        quex_coding_name_str = { 1: "ASCII", 2: "UCS-2LE", 4: "UCS-4LE", 
                                    "wchar_t": "WCHAR_T" }[Setup.bytes_per_ucs_code_point]
    else:
        quex_coding_name_str = { 1: "ASCII", 2: "UCS-2BE", 4: "UCS-4BE", 
                                    "wchar_t": "WCHAR_T" }[Setup.bytes_per_ucs_code_point]


    # -- determine whether the lexical analyser needs indentation counting
    #    support. if one mode has an indentation handler, than indentation
    #    support must be provided.
    indentation_support_f = False
    for mode in Modes.values():
        if mode.on_indentation.get_code() != "":
            indentation_support_f = True
            break

    lex_id_definitions_str = "" 
    # NOTE: First mode-id needs to be '1' for compatibility with flex generated engines
    i = 0
    for name in Modes.keys():
        i += 1
        lex_id_definitions_str += "const int LEX_ID_%s = %i;\n" % (name, i)

    include_guard_extension = get_include_guard_extension(Setup.output_file_stem)

    # -- instances of mode classes as members of the lexer
    mode_object_members_txt,     \
    constructor_txt,             \
    mode_specific_functions_txt, \
    friend_txt =                 \
         get_mode_class_related_code_fragments(Modes.values(), LexerClassName)

    # -- define a pointer that directly has the type of the derived class
    if Setup.input_derived_class_name == "":
        Setup.input_derived_class_name = LexerClassName
        derived_class_type_declaration = ""
    else:
        derived_class_type_declaration = "class %s;" % Setup.input_derived_class_name

    # -- the friends of the class
    friends_str = ""
    for friend in Setup.input_lexer_class_friends:
        friends_str += "    friend class %s;\n" % friend

    # -- the class body extension
    class_body_extension_str = lexer_mode.class_body.get_code()

    # -- the class constructor extension
    class_constructor_extension_str = lexer_mode.class_init.get_code()

    fh = open_file_or_die(QuexClassHeaderFileTemplate)
    template_code_txt = fh.read()
    fh.close()

    # -- check if exit/entry handlers have to be active
    entry_handler_active_f = False
    exit_handler_active_f = False
    for mode in Modes.values():
        if mode.on_entry_code_fragments() != []: entry_handler_active_f = True
        if mode.on_exit_code_fragments() != []:  exit_handler_active_f = True

    txt = template_code_txt
    def set_switch(txt, SwitchF, Name):
        if SwitchF: txt = txt.replace("$$SWITCH$$ %s" % Name, "#define    %s" % Name)
        else:       txt = txt.replace("$$SWITCH$$ %s" % Name, "// #define %s" % Name)
        return txt
    
    txt = set_switch(txt, entry_handler_active_f,  "__QUEX_OPTION_ON_ENTRY_HANDLER_PRESENT")
    txt = set_switch(txt, exit_handler_active_f,   "__QUEX_OPTION_ON_EXIT_HANDLER_PRESENT")
    txt = set_switch(txt, indentation_support_f,   "__QUEX_OPTION_INDENTATION_TRIGGER_SUPPORT")     
    txt = set_switch(txt, True,                    "__QUEX_OPTION_SUPPORT_BEGIN_OF_LINE_PRE_CONDITION")
    txt = set_switch(txt, Setup.enable_iconv_f,    "QUEX_OPTION_ENABLE_ICONV")
    txt = set_switch(txt, not Setup.disable_token_queue_f,        "QUEX_OPTION_TOKEN_SENDING_VIA_QUEUE")
    txt = set_switch(txt, not Setup.disable_string_accumulator_f, "QUEX_OPTION_STRING_ACCUMULATOR")
    txt = set_switch(txt, Setup.post_categorizer_f,               "QUEX_OPTION_POST_CATEGORIZER")
    txt = set_switch(txt, True,                    "QUEX_OPTION_VIRTUAL_FUNCTION_ON_ACTION_ENTRY")      
    txt = set_switch(txt, True,                    "QUEX_OPTION_LINE_NUMBER_COUNTING")      
    txt = set_switch(txt, True,                    "QUEX_OPTION_COLUMN_NUMBER_COUNTING")        
    txt = set_switch(txt, Setup.output_debug_f,    "QUEX_OPTION_DEBUG_TOKEN_SENDING")
    txt = set_switch(txt, Setup.output_debug_f,    "QUEX_OPTION_DEBUG_MODE_TRANSITIONS")
    txt = set_switch(txt, Setup.output_debug_f,    "QUEX_OPTION_DEBUG_QUEX_PATTERN_MATCHES")
    txt = set_switch(txt, True,                    "QUEX_OPTION_INCLUDE_STACK_SUPPORT")
    txt = set_switch(txt, not Setup.no_mode_transition_check_f,           
                               "QUEX_OPTION_RUNTIME_MODE_TRANSITION_CHECK")

    txt = blue_print(txt,
            [
                ["$$BUFFER_LIMIT_CODE$$",            "0x%X" % Setup.buffer_limit_code],
                ["$$CONSTRUCTOR_EXTENSTION$$",                  class_constructor_extension_str],
                ["$$CONSTRUCTOR_MODE_DB_INITIALIZATION_CODE$$", constructor_txt],
                ["$$CORE_ENGINE_DEFINITIONS_HEADER$$",          CoreEngineDefinitionsHeader],
                ["$$CLASS_BODY_EXTENSION$$",         class_body_extension_str],
                ["$$INCLUDE_GUARD_EXTENSION$$",      include_guard_extension],
                ["$$INITIAL_LEXER_MODE_ID$$",        "LEX_ID_" + lexer_mode.initial_mode.get_code()],
                ["$$LEXER_BUILD_DATE$$",             time.asctime()],
                ["$$LEXER_BUILD_VERSION$$",          VersionID],
                ["$$LEXER_CLASS_FRIENDS$$",          friends_str],
                ["$$LEXER_CLASS_NAME$$",             LexerClassName],
                ["$$LEXER_DERIVED_CLASS_DECL$$",     derived_class_type_declaration],
                ["$$LEXER_DERIVED_CLASS_NAME$$",     Setup.input_derived_class_name],
                ["$$LEX_ID_DEFINITIONS$$",           lex_id_definitions_str],
                ["$$MAX_MODE_CLASS_N$$",             repr(len(Modes))],
                ["$$MODE_CLASS_FRIENDS$$",           friend_txt],
                ["$$MODE_OBJECT_MEMBERS$$",              mode_object_members_txt],
                ["$$MODE_SPECIFIC_ANALYSER_FUNCTIONS$$", mode_specific_functions_txt],
                ["$$PRETTY_INDENTATION$$",               "     " + " " * (len(LexerClassName)*2 + 2)],
                ["$$QUEX_TEMPLATE_DIR$$",                Setup.QUEX_TEMPLATE_DB_DIR],
                ["$$QUEX_VERSION$$",                     QuexVersionID],
                ["$$TOKEN_CLASS$$",                      Setup.input_token_class_name],
                ["$$TOKEN_CLASS_DEFINITION_FILE$$",      Setup.input_token_class_file.replace("//","/")],
                ["$$TOKEN_ID_DEFINITION_FILE$$",         Setup.output_token_id_file.replace("//","/")],
                ["$$QUEX_CHARACTER_TYPE$$",              quex_character_type_str],
                ["$$QUEX_LEXEME_TYPE$$",                 quex_lexeme_type_str],
                ["$$CORE_ENGINE_CHARACTER_CODING$$",     quex_coding_name_str],
                ["$$USER_DEFINED_HEADER$$",              lexer_mode.header.get_code() + "\n"],
             ])

    fh_out = open(QuexClassHeaderFileOutput, "wb")
    if os.linesep != "\n": txt = txt.replace("\n", os.linesep)
    fh_out.write(txt)
    fh_out.close()


def write_mode_class_implementation(Modes, Setup):
    LexerClassName              = Setup.output_engine_name
    TokenClassName              = Setup.input_token_class_name
    OutputFilestem              = Setup.output_file_stem
    DerivedClassName            = Setup.input_derived_class_name
    DerivedClassHeaderFileName  = Setup.input_derived_class_file
    ModeClassImplementationFile = Setup.output_code_file

    if DerivedClassHeaderFileName != "": txt = "#include<" + DerivedClassHeaderFileName +">\n"
    else:                                txt = "#include\"" + OutputFilestem +"\"\n"
    
    # -- mode class member function definitions (on_entry, on_exit, has_base, ...)
    mode_class_member_functions_txt = mode_classes.do(Modes.values())

    mode_objects_txt = ""    
    for mode_name in Modes:
        mode_objects_txt += "        QuexMode  $$LEXER_CLASS_NAME$$::%s;\n" % mode_name

    txt += "namespace quex {\n"
    txt += mode_objects_txt
    txt += mode_class_member_functions_txt
    txt += "} // END: namespace quex\n"

    txt = blue_print(txt, [["$$LEXER_CLASS_NAME$$",         LexerClassName],
                          ["$$TOKEN_CLASS$$",              TokenClassName],
                          ["$$LEXER_DERIVED_CLASS_NAME$$", DerivedClassName]])
    
    fh_out = open(ModeClassImplementationFile, "wb")
    if os.linesep != "\n": txt = txt.replace("\n", os.linesep)
    fh_out.write(txt)
    fh_out.close()


quex_mode_init_call_str = """
        $$MN$$.id   = LEX_ID_$$MN$$;
        $$MN$$.name = "$$MN$$";
        $$MN$$.analyser_function = $analyser_function;
#    ifdef __QUEX_OPTION_INDENTATION_TRIGGER_SUPPORT    
        $$MN$$.on_indentation = $on_indentation;
#    endif
        $$MN$$.on_entry       = $on_entry;
        $$MN$$.on_exit        = $on_exit;
#    ifdef __QUEX_OPTION_RUNTIME_MODE_TRANSITION_CHECK
        $$MN$$.has_base       = $has_base;
        $$MN$$.has_entry_from = $has_entry_from;
        $$MN$$.has_exit_to    = $has_exit_to;
#    endif
"""

def __get_mode_init_call(mode, LexerClassName):
    
    header_str = "%s_%s_" % (LexerClassName, mode.name)

    analyser_function = header_str + "analyser_function" 
    on_indentation    = header_str + "on_indentation"    
    on_entry          = header_str + "on_entry"          
    on_exit           = header_str + "on_exit"           
    has_base          = header_str + "has_base"          
    has_entry_from    = header_str + "has_entry_from"    
    has_exit_to       = header_str + "has_exit_to"       

    if mode.options["inheritable"] == "only": 
        analyser_function = "QuexMode_uncallable_analyser_function"

    if mode.on_entry_code_fragments() == []:
        on_entry = "QuexMode_on_entry_exit_null_function"

    if mode.on_exit_code_fragments() == []:
        on_exit = "QuexMode_on_entry_exit_null_function"

    if mode.on_indentation_code_fragments() == []:
        on_indentation = "QuexMode_on_indentation_null_function"

    txt = blue_print(quex_mode_init_call_str,
                [["$$MN$$",             mode.name],
                 ["$analyser_function", analyser_function],
                 ["$on_indentation",    on_indentation],
                 ["$on_entry",          on_entry],
                 ["$on_exit",           on_exit],
                 ["$has_base",          has_base],
                 ["$has_entry_from",    has_entry_from],
                 ["$has_exit_to",       has_exit_to]])

    return txt

def __get_mode_function_declaration(Modes, LexerClassName, FriendF=False):

    if FriendF: prolog = "        friend "
    else:       prolog = "    extern "

    def __mode_functions(Prolog, ReturnType, NameList, ArgList):
        txt = ""
        for name in NameList:
            function_signature = "%s %s_%s_%s(%s);" % \
                     (ReturnType, LexerClassName, mode.name, name, ArgList)
            txt += "%s" % Prolog + "    " + function_signature + "\n"

        return txt

    txt = ""
    for mode in Modes:
        if mode.options["inheritable"] != "only":
            txt += __mode_functions(prolog, "__QUEX_SETTING_ANALYSER_FUNCTION_RETURN_TYPE", ["analyser_function"],
                                    "QuexAnalyser*")
    for mode in Modes:
        if mode.on_indentation_code_fragments() != []:
            txt += __mode_functions(prolog, "void", ["on_indentation"], 
                                    LexerClassName + "*, const int")

    for mode in Modes:
        if mode.on_entry_code_fragments() != []:
            txt += __mode_functions(prolog, "void", ["on_entry"], 
                                    LexerClassName + "*, const QuexMode*")

        if mode.on_exit_code_fragments() != []:
            txt += __mode_functions(prolog, "void", ["on_exit"], 
                                    LexerClassName + "*, const QuexMode*")

    txt += "#ifdef __QUEX_OPTION_RUNTIME_MODE_TRANSITION_CHECK\n"
    for mode in Modes:
        txt += __mode_functions(prolog, "bool", ["has_base", "has_entry_from", "has_exit_to"], 
                                "const QuexMode*")
        
    txt += "#endif\n"
    txt += "\n"

    return txt

def get_mode_class_related_code_fragments(Modes, LexerClassName):
    """
       RETURNS:  -- members of the lexical analyzer class for the mode classes
                 -- static member functions declaring the analyzer functions for he mode classes 
                 -- constructor init expressions (before '{'),       
                 -- constructor text to be executed at construction time 
                 -- friend declarations for the mode classes/functions

    """

    L = max(map(lambda m: len(m.name), Modes))

    members_txt = ""    
    for mode in Modes:
        members_txt += "        static QuexMode  %s;\n" % mode.name

    # constructor code
    txt = ""
    for mode in Modes:
        txt += "        __quex_assert(LEX_ID_%s %s<= %i);\n" % (mode.name, " " * (L-len(mode.name)), len(Modes))

    for mode in Modes:
        txt += __get_mode_init_call(mode, LexerClassName)

    for mode in Modes:
        txt += "        mode_db[LEX_ID_%s]%s = &%s;\n" % (mode.name, " " * (L-len(mode.name)), mode.name)

    constructor_txt = txt

    mode_functions_txt = __get_mode_function_declaration(Modes, LexerClassName, FriendF=False)
    friends_txt        = __get_mode_function_declaration(Modes, LexerClassName, FriendF=True)

    return members_txt,        \
           constructor_txt,    \
           mode_functions_txt, \
           friends_txt
