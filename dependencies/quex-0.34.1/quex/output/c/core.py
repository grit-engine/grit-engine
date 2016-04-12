#! /usr/bin/env python
from copy import copy
import time

from quex.frs_py.string_handling import blue_print
from quex.frs_py.file_in         import open_file_or_die, \
                                        get_include_guard_extension

import quex.lexer_mode            as lexer_mode
import quex.output.c.mode_classes as mode_classes


def do(Modes, setup):

    QuexClassHeaderFileTemplate = (setup.QUEX_TEMPLATE_DB_DIR 
                               + "/template/lexical_analyzer_class-C").replace("//","/")
    CoreEngineDefinitionsHeader = (setup.QUEX_TEMPLATE_DB_DIR + "/core_engine/").replace("//","/")
    if setup.plain_memory_f: CoreEngineDefinitionsHeader += "definitions-quex-buffer.h"
    else:                    CoreEngineDefinitionsHeader += "definitions-plain-memory.h"
    QuexClassHeaderFileOutput   = setup.output_file_stem
    LexerClassName              = setup.output_engine_name
    VersionID                   = setup.input_application_version_id
    QuexVersionID               = setup.QUEX_VERSION
    DerivedClassHeaderFileName  = setup.input_derived_class_file
    ModeClassImplementationFile = setup.output_code_file


    # -- determine whether the lexical analyser needs indentation counting
    #    support. if one mode has an indentation handler, than indentation
    #    support must be provided.
    indentation_support_f = False
    for mode in Modes.values():
        if mode.on_indentation.line_n != -1:
        indentation_support_f = True
            break

    lex_id_definitions_str = "" 
    # NOTE: First mode-id needs to be '1' for compatibility with flex generated engines
    i = 0
    for name in Modes.keys():
    i += 1
    lex_id_definitions_str += "const int LEX_ID_%s = %i;\n" % (name, i)

    include_guard_extension = get_include_guard_extension(setup.output_file_stem)

    # -- mode class member function definitions (on_entry, on_exit, has_base, ...)
    mode_class_member_functions_txt = mode_classes.do(Modes.values())

    # -- instances of mode classes as members of the lexer
    mode_object_members_txt,     \
    constructor_txt,             \
    mode_specific_functions_txt, \
    friend_txt =                 \
         get_mode_class_related_code_fragments(Modes.values())

    # -- get the code for the user defined all-match actions
    try:
        fh_aux = open(setup.output.user_match_action)
        user_match_action_str = fh_aux.read()
        fh_aux.close()
    except:
        user_match_action_str = "/* no extra class content */"

    # -- define a pointer that directly has the type of the derived class
    if setup.input_derived_class_name == "":
        setup.input_derived_class_name = LexerClassName
        derived_class_type_declaration = ""
    else:
        derived_class_type_declaration = "class %s;" % setup.input_derived_class_name

    # -- the friends of the class
    friends_str = ""
    for friend in setup.input_lexer_class_friends:
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
    if SwitchF: txt = txt.replace("%%%%SWITCH%%%% %s" % Name, "#define    %s" % Name)
    else:       txt = txt.replace("%%%%SWITCH%%%% %s" % Name, "// #define %s" % Name)
    return txt
    
    txt = set_switch(txt, entry_handler_active_f,  "__QUEX_OPTION_ON_ENTRY_HANDLER_PRESENT")
    txt = set_switch(txt, exit_handler_active_f,   "__QUEX_OPTION_ON_EXIT_HANDLER_PRESENT")
    txt = set_switch(txt, indentation_support_f,   "__QUEX_OPTION_INDENTATION_TRIGGER_SUPPORT")     
    txt = set_switch(txt, setup.plain_memory_f,    "__QUEX_CORE_OPTION_PLAIN_MEMORY_BASED")     
    txt = set_switch(txt, True,                    "__QUEX_CORE_OPTION_SUPPORT_BEGIN_OF_LINE_PRE_CONDITION")
    txt = set_switch(txt, True,                    "QUEX_OPTION_VIRTUAL_FUNCTION_ON_ACTION_ENTRY")      
    txt = set_switch(txt, False,                   "QUEX_OPTION_NO_LINE_NUMBER_COUNTING")       
    txt = set_switch(txt, False,                   "QUEX_OPTION_NO_COLUMN_NUMBER_COUNTING")     
    
    txt = blue_print(txt,
                     [
                      ["%%CONSTRUCTOR_EXTENSTION%%",                  class_constructor_extension_str],
                      ["%%CONSTRUCTOR_MODE_DB_INITIALIZATION_CODE%%", constructor_txt],
                      ["%%CORE_ENGINE_DEFINITIONS_HEADER%%",          CoreEngineDefinitionsHeader],
                      ["%%CLASS_BODY_EXTENSION%%",         class_body_extension_str],
                      ["%%INCLUDE_GUARD_EXTENSION%%",      include_guard_extension],
                      ["%%INITIAL_LEXER_MODE_ID%%",        "LEX_ID_" + lexer_mode.initial_mode.get_code()],
                      ["%%LEXER_BUILD_DATE%%",             time.asctime()],
                      ["%%LEXER_BUILD_VERSION%%",          VersionID],
                      ["%%LEXER_CLASS_FRIENDS%%",          friends_str],
                      ["$$LEXER_CLASS_NAME$$",             LexerClassName],
                      ["%%LEXER_DERIVED_CLASS_DECL%%",     derived_class_type_declaration],
                      ["%%LEXER_DERIVED_CLASS_NAME%%",     setup.input_derived_class_name],
                      ["%%LEX_ID_DEFINITIONS%%",           lex_id_definitions_str],
                      ["%%MAX_MODE_CLASS_N%%",             repr(len(Modes))],
                      ["%%MODE_CLASS_FRIENDS%%",           friend_txt],
                      ["%%MODE_OBJECT_MEMBERS%%",          mode_object_members_txt],
                      ["%%MODE_SPECIFIC_ANALYSER_FUNCTIONS%%", mode_specific_functions_txt],
                      ["%%PRETTY_INDENTATION%%",           "     " + " " * (len(LexerClassName)*2 + 2)],
                      ["%%QUEX_TEMPLATE_DIR%%",            setup.QUEX_TEMPLATE_DB_DIR],
                      ["%%QUEX_VERSION%%",                 QuexVersionID],
                      ["%%TOKEN_CLASS%%",                  setup.input_token_class_name],
                      ["%%TOKEN_CLASS_DEFINITION_FILE%%",  setup.input_token_class_file.replace("//","/")],
                      ["%%TOKEN_ID_DEFINITION_FILE%%",     setup.output_token_id_file.replace("//","/")],
                      ["%%QUEX_OUTPUT_FILESTEM%%",         setup.output_file_stem],
             ])

    fh_out = open(QuexClassHeaderFileOutput, "w")
    fh_out.write(txt)
    fh_out.close()

    fh_out = open(ModeClassImplementationFile, "w")
    fh_out.write(lexer_mode.header.get() + "\n")
    
    if DerivedClassHeaderFileName != "":
    fh_out.write("#include<" + DerivedClassHeaderFileName +">\n")
    else:
    fh_out.write("#include<" + setup.output_file_stem +">\n")
    
    fh_out.write("namespace quex {\n")

    mode_class_member_functions_txt = \
         blue_print(mode_class_member_functions_txt,
                [["$$LEXER_CLASS_NAME$$",         LexerClassName],
                             ["%%TOKEN_CLASS%%",              setup.input_token_class_name],
                 ["%%LEXER_DERIVED_CLASS_NAME%%", setup.input_derived_class_name]])
    
    fh_out.write(mode_class_member_functions_txt)
    fh_out.write("} // END: namespace quex\n")
    fh_out.close()


quex_mode_init_call_str = """
        quex_mode_init(&%%MN%%, this, 
                   LEX_ID_%%MN%%, "%%MN%%",
               $analyser_function,
#ifdef __QUEX_OPTION_INDENTATION_TRIGGER_SUPPORT    
               $on_indentation,
#endif
               $on_entry,
               $on_exit
#ifdef __QUEX_OPTION_RUNTIME_MODE_TRANSITION_CHECK
               ,
               $has_base,
               $has_entry_from,
               $has_exit_to
#endif
                      );
"""

def __get_mode_init_call(mode):

    analyser_function = "$$LEXER_CLASS_NAME$$__%s_analyser_function" % mode.name
    on_indentation    = "$$LEXER_CLASS_NAME$$__%s_on_indentation"    % mode.name
    on_entry          = "$$LEXER_CLASS_NAME$$__%s_on_entry"          % mode.name
    on_exit           = "$$LEXER_CLASS_NAME$$__%s_on_exit"           % mode.name
    has_base          = "$$LEXER_CLASS_NAME$$__%s_has_base"          % mode.name
    has_entry_from    = "$$LEXER_CLASS_NAME$$__%s_has_entry_from"    % mode.name
    has_exit_to       = "$$LEXER_CLASS_NAME$$__%s_has_exit_to"       % mode.name

    if mode.options["inheritable"] == "only": 
    analyser_function = "/* %s = */ 0x0" % analyser_function

    if mode.on_entry_code_fragments() == []:
    on_entry = "/* %s = */ $$LEXER_CLASS_NAME$$_on_entry_exit_null_function" % on_entry

    if mode.on_exit_code_fragments() == []:
    on_exit = "/* %s = */ $$LEXER_CLASS_NAME$$_on_entry_exit_null_function" % on_exit

    if mode.on_indentation_code_fragments() == []:
    on_indentation = "/* %s = */ 0x0" % on_indentation

    txt = blue_print(quex_mode_init_call_str,
                [["%%MN%%",             mode.name],
             ["$analyser_function", analyser_function],
                 ["$on_indentation",    on_indentation],
                 ["$on_entry",          on_entry],
                 ["$on_exit",           on_exit],
                 ["$has_base",          has_base],
                 ["$has_entry_from",    has_entry_from],
                 ["$has_exit_to",       has_exit_to]])
    return txt



def __get_mode_function_declaration(Modes, FriendF=False):
    if FriendF: prolog = "        friend "
    else:       prolog = "    extern "

    def __mode_functions(Prolog, ReturnType, NameList, ArgList):
    txt = ""
    for name in NameList:
        function_signature = "%s $$LEXER_CLASS_NAME$$__%s_%s(%s);" % \
                 (ReturnType, mode.name, name, ArgList)
        txt += "%s" % Prolog + "    " + function_signature + "\n"
    return txt

    txt = ""
    for mode in Modes:
        if mode.options["inheritable"] != "only": 
            txt += __mode_functions(prolog, 
                                    "__QUEX_SETTING_ANALYSER_FUNCTION_RETURN_TYPE", ["analyser_function"], 
                                    "$$LEXER_CLASS_NAME$$*")

    for mode in Modes:
        if mode.on_indentation_code_fragments() != []:
            txt += __mode_functions(prolog, "void", ["on_indentation"], 
                            "$$LEXER_CLASS_NAME$$*, const quex_mode*")

    for mode in Modes:
    if mode.on_entry_code_fragments() != []:
        txt += __mode_functions(prolog, "void", ["on_entry"], 
                    "$$LEXER_CLASS_NAME$$*, const quex_mode*")
    if mode.on_exit_code_fragments() != []:
        txt += __mode_functions(prolog, "void", ["on_exit"], 
                    "$$LEXER_CLASS_NAME$$*, const quex_mode*")

    txt += "#ifdef __QUEX_OPTION_RUNTIME_MODE_TRANSITION_CHECK\n"
    for mode in Modes:
    txt += __mode_functions(prolog, "bool", ["has_base", "has_entry_from", "has_exit_to"], 
                    "const quex_mode*")
        
    txt += "#endif\n"
    txt += "\n"

    return txt


def get_mode_class_related_code_fragments(Modes):
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
        members_txt += "        quex_mode  %s;\n" % mode.name


    # constructor code
    txt = ""
    for mode in Modes:
        txt += "        assert(LEX_ID_%s %s<= %i);\n" % (mode.name, " " * (L-len(mode.name)), len(Modes))

    for mode in Modes:
    txt += __get_mode_init_call(mode)

    for mode in Modes:
        txt += "        mode_db[LEX_ID_%s]%s = &%s;\n" % (mode.name, " " * (L-len(mode.name)), mode.name)
    constructor_txt = txt

    mode_functions_txt = __get_mode_function_declaration(Modes, FriendF=False)
    friends_txt        = __get_mode_function_declaration(Modes, FriendF=True)

    return members_txt,        \
       constructor_txt,    \
       mode_functions_txt, \
       friends_txt
