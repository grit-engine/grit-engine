import quex.core_engine.state_machine.character_counter as pattern_analyzer
from   quex.core_engine.interval_handling     import NumberSet
from   quex.core_engine.generator.action_info import *
from   quex.input.setup                       import setup as Setup

def do(Mode, CodeFragment_or_CodeFragments, SafePatternStr, PatternStateMachine, 
       Default_ActionF=False, EOF_ActionF=False):
    """-- If there are multiple handlers for a single event they are combined
    
       -- Adding debug information printer (if desired)
    
       -- The task of this function is it to adorn the action code for each pattern with
          code for line and column number counting.
    """
    assert Mode.__class__.__name__                == "LexMode"
    assert type(SafePatternStr)                   == str
    assert PatternStateMachine.__class__.__name__ == "StateMachine" or PatternStateMachine == None
    assert type(Default_ActionF)                  == bool
    assert type(EOF_ActionF)                      == bool

    if type(CodeFragment_or_CodeFragments) == list:
        assert Default_ActionF or EOF_ActionF, \
               "Action code formatting: Multiple Code Fragments can only be specified for default or\n" + \
               "end of stream action."
        CodeFragmentList = CodeFragment_or_CodeFragments
    else:
        CodeFragmentList = [ CodeFragment_or_CodeFragments ]

    on_every_match_code = ""
    lc_count_code       = ""
    debug_code          = ""
    user_code           = ""

    # (*) Code to be performed on every match -- before the related action
    for code_info in Mode.on_match_code_fragments():
        on_every_match_code += code_info.get_code()

    # (*) Code to count line and column numbers
    if Mode.on_indentation.get_code() != "":
        lc_count_code = __get_line_and_column_counting_with_indentation(PatternStateMachine, EOF_ActionF)

    else:
        lc_count_code = __get_line_and_column_counting(PatternStateMachine, EOF_ActionF)

    # (*) debug prints -- if desired
    if Setup.output_debug_f == True:
        txt  = '#ifdef QUEX_OPTION_DEBUG_QUEX_PATTERN_MATCHES\n'
        txt += '    std::cerr << "(" << self.line_number_at_begin() << ", " << self.column_number_at_begin()'
        txt += '<< ") %s: %s \'" << Lexeme << "\'\\n";\n' % (Mode.name, SafePatternStr)
        txt += '#endif\n'
        debug_code = txt
        
    # (*) THE user defined action to be performed in case of a match
    require_terminating_zero_preparation_f = False
    for code_info in CodeFragmentList:
        user_code += code_info.get_code()
        if code_info.require_terminating_zero_f():
            require_terminating_zero_preparation_f = True

    txt  = "{\n"
    txt += on_every_match_code
    txt += lc_count_code
    txt += debug_code
    txt += user_code
    txt += "\n}"

    return CodeFragment(txt, require_terminating_zero_preparation_f)


def __get_line_and_column_counting_with_indentation(PatternStateMachine, EOF_ActionF):

    # shift the values for line and column numbering
    txt = "self.counter.__shift_end_values_to_start_values();\n"

    if EOF_ActionF:
        txt += "#ifdef __QUEX_OPTION_INDENTATION_TRIGGER_SUPPORT\n"
        txt += "    self.counter.on_end_of_file();\n"
        txt += "#endif\n"
        return txt

    if PatternStateMachine == None:
        return txt + "self.counter.icount(Lexeme, LexemeEnd);\n"

    newline_n   = pattern_analyzer.get_newline_n(PatternStateMachine)
    character_n = pattern_analyzer.get_character_n(PatternStateMachine)

    # later implementations may consider '\t' also for indentation counting
    whitespace_set  = NumberSet(ord(' '))
    initial_triggers = PatternStateMachine.get_init_state().transitions().get_trigger_set_union()

    starts_never_on_whitespace_f = not initial_triggers.has_intersection(whitespace_set)
    contains_only_spaces_f       = pattern_analyzer.contains_only_spaces(PatternStateMachine)

    if newline_n != 0:
        # IDEA: (case newline_n > 0) 
        #       Try to determine number of characters backwards to newline directly
        #       from the pattern state machine.
        func = "self.counter.icount(Lexeme, LexemeEnd);"       

    else:
        if character_n == -1: column_increment = "LexemeL"          # based on matched lexeme
        else:                 column_increment = "%i" % character_n # fixed length
            
        if starts_never_on_whitespace_f:
            func = "self.counter.icount_NoNewline_NeverStartOnWhitespace(%s);" % column_increment
        elif contains_only_spaces_f:
            func = "self.counter.icount_NoNewline_ContainsOnlySpaces(%s);" % column_increment
        else:
            func = "self.counter.icount_NoNewline(Lexeme, LexemeEnd);"

    return txt + func + "\n"

def __get_line_and_column_counting(PatternStateMachine, EOF_ActionF):

    # shift the values for line and column numbering
    txt = "self.counter.__shift_end_values_to_start_values();\n"

    if EOF_ActionF:
        return txt

    if PatternStateMachine == None:
        return txt + "self.counter.count(Lexeme, LexemeEnd);\n"

    newline_n   = pattern_analyzer.get_newline_n(PatternStateMachine)
    character_n = pattern_analyzer.get_character_n(PatternStateMachine)

    if   newline_n == -1:
        # run the general algorithm, since not even the number of newlines in the 
        # pattern can be determined directly from the pattern
        return txt + "self.counter.count(Lexeme, LexemeEnd);\n"

    elif newline_n != 0:
        # TODO: Try to determine number of characters backwards to newline directly
        #       from the pattern state machine. (Those seldom cases won't bring much
        #       speed-up)
        return txt + "self.counter.count_FixNewlineN(Lexeme, LexemeEnd, %i);\n" % newline_n

    else:
        if character_n == -1: incr_str = "LexemeL"
        else:                 incr_str = repr(character_n) 

        return txt + "self.counter.count_NoNewline(%s);\n" % incr_str

