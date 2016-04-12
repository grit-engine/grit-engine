from   quex.frs_py.string_handling import blue_print


def do(Modes):

    # -- get the implementation of mode class functions
    #    (on_entry, on_exit, on_indendation, has_base, has_entry, has_exit)
    txt = "#define self  (*me)\n"
    txt += "void $$LEXER_CLASS_NAME$$_on_entry_exit_null_function($$LEXER_CLASS_NAME$$* me, const quex_mode* Mode)\n"
    txt += "{}\n\n"

    for mode in Modes:        
        txt += get_implementation_of_mode_functions(mode, Modes)

    txt += "#undef self\n"
    return txt


mode_function_implementation_str = \
"""
    void
    $$LEXER_CLASS_NAME$$__%%MODE_NAME%%_on_entry($$LEXER_CLASS_NAME$$* me, const quex_mode* FromMode) {
%%ENTER-PROCEDURE%%
    }
    void
    $$LEXER_CLASS_NAME$$__%%MODE_NAME%%_on_exit($$LEXER_CLASS_NAME$$* me, const quex_mode* ToMode)  {
%%EXIT-PROCEDURE%%
    }

#ifdef __QUEX_OPTION_INDENTATION_TRIGGER_SUPPORT        
    void
    $$LEXER_CLASS_NAME$$__%%MODE_NAME%%_on_indentation($$LEXER_CLASS_NAME$$* me, const int Indentation) {
%%INDENTATION-PROCEDURE%%
    }
#endif

#ifdef __QUEX_OPTION_RUNTIME_MODE_TRANSITION_CHECK
    bool
    $$LEXER_CLASS_NAME$$__%%MODE_NAME%%_has_base(const quex_mode* Mode) {
%%HAS_BASE_MODE%%
    }
    bool
    $$LEXER_CLASS_NAME$$__%%MODE_NAME%%_has_entry_from(const quex_mode* Mode) {
%%HAS_ENTRANCE_FROM%%
    }
    bool
    $$LEXER_CLASS_NAME$$__%%MODE_NAME%%_has_exit_to(const quex_mode* Mode) {
%%HAS_EXIT_TO%%
    }
#endif    
"""                         

def  get_implementation_of_mode_functions(mode, Modes):
    """Writes constructors and mode transition functions.

                  void quex::lexer::enter_EXAMPLE_MODE() { ... }

       where EXAMPLE_MODE is a lexer mode from the given lexer-modes, and
       'quex::lexer' is the lexical analysis class.
    """
    # (*) on enter 
    code_fragments = mode.on_entry_code_fragments()    
    on_entry_str  = "#ifdef __QUEX_OPTION_RUNTIME_MODE_TRANSITION_CHECK\n"
    on_entry_str += "assert(me->%s.has_entry_from(FromMode));\n" % mode.name
    on_entry_str += "#endif\n"
    for code_info in code_fragments:
        on_entry_str += code_info.get_code()
        if on_entry_str[-1] == "\n": on_entry_str = on_entry_str[:-1]

    # (*) on exit
    code_fragments = mode.on_exit_code_fragments()    
    on_exit_str  = "#ifdef __QUEX_OPTION_RUNTIME_MODE_TRANSITION_CHECK\n"
    on_exit_str += "assert(me->%s.has_exit_to(ToMode));\n" % mode.name
    on_exit_str += "#endif\n"
    for code_info in code_fragments:
        on_exit_str += code_info.get_code()

    # (*) on indentation
    code_fragments = mode.on_indentation_code_fragments()    
    on_indentation_str = "assert(Indentation >= 0);" 
    for code_info in code_fragments:
        on_indentation_str += code_info.get_code()
        
    # (*) has base mode
    if mode.get_base_modes() != []:
        has_base_mode_str = get_IsOneOfThoseCode(mode.get_base_modes())
    else:
        has_base_mode_str = "    return false;"
        
    # (*) has entry from
    try:
        entry_list = mode.options["entry"]        
        has_entry_from_str = get_IsOneOfThoseCode(entry_list,
                                                  ConsiderDerivedClassesF=true)
        # check whether the mode we come from is an allowed mode
    except:
        has_entry_from_str = "    return true; // default"        

    # (*) has exit to
    try:
        exit_list = mode.options["exit"]
        has_exit_to_str = get_IsOneOfThoseCode(exit_list,
                                               ConsiderDerivedClassesF=true)
    except:
        has_exit_to_str = "    return true; // default"

    
    txt = blue_print(mode_function_implementation_str,
                     [["%%ENTER-PROCEDURE%%",           on_entry_str],
                      ["%%EXIT-PROCEDURE%%",            on_exit_str],
                      ["%%INDENTATION-PROCEDURE%%",     on_indentation_str],
                      ["%%HAS_BASE_MODE%%",             has_base_mode_str],
                      ["%%HAS_ENTRANCE_FROM%%",         has_entry_from_str],
                      ["%%HAS_EXIT_TO%%",               has_exit_to_str],
                      ["%%MODE_NAME%%",                 mode.name],
                      ])
    return txt



def get_IsOneOfThoseCode(ThoseModes, Indentation="    ",
                         ConsiderDerivedClassesF=False):
    txt = Indentation
    if ThoseModes == []:
        return Indentation + "return false;"
    

    # NOTE: Usually, switch commands do a binary bracketting.
    #       (Cannot be faster here ... is not critical anyway since this is debug code)
    txt  = "\n"
    txt += "switch( Mode->id ) {\n"
    for mode_name in ThoseModes:
        txt += "case LEX_ID_%s: return true;\n" % mode_name
    txt += "default:\n"
    if ConsiderDerivedClassesF:
        for mode_name in ThoseModes:
            txt += "    if( Mode->has_base(%s) ) return true;\n" % mode_name
    else:
        txt += ";\n"
    txt += "}\n"

    txt += "#ifdef __QUEX_OPTION_ERROR_OUTPUT_ON_MODE_CHANGE_ERROR\n"
    if ConsiderDerivedClassesF:
        txt += "std::cerr << \"mode '%s' is not one of (and not a a derived mode of):\\n\";\n" % mode_name
    else:
        txt += "std::cerr << \"mode '%s' is not one of:\\n\";" % mode_name
    for mode_name in ThoseModes:
            txt += "    std::cerr << \"         %s\\n\";\n" % mode_name
    txt += "#endif // QUEX_OPTION_ERROR_OUTPUT_ON_MODE_CHANGE_ERROR\n"
    txt += "return false;\n"

    return txt.replace("\n", "\n" + Indentation)

