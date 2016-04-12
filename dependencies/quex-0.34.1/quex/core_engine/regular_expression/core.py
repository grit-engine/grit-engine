# The 'grammar' of quex's regular expressions:
#
#  complete expression: expression
#                       expression / expression                 = post conditioned expression
#                       expression / expression /               = pre conditioned expression
#                       expression / expression / expression    = pre and post conditioned expression
# 
#  expression: term
#              term | expression
#  
#  term:  primary
#         primary term
#  
#  primary:  " non_double_quote *  "              = character string
#            [ non_rect_bracket_close ]           = set of characters
#            { identifier }                       = pattern replacement
#            ( expression )
#            non_control_character+               = lonely characters
#            primary repetition_cmd
#  
#  non_double_quote: 'anything but an unbackslashed double quote, i.e. \" is ok, 
#                     but " is not.'      
#  non_rect_bracket_close: 'anything but an unbackslashed rectangular bracket, i.e.
#                           \] is ok, but ] is not.'               
#  non_control_character: 'anything but (, ", [, or {'
#       
#  repetition_cmd: 'a repetition command such as +, *, {2}, {,2}, {2,5}'        
#
#########################################################################################       
import sys
import StringIO

from quex.frs_py.file_in  import *

from quex.exception                      import RegularExpressionException
from quex.core_engine.interval_handling  import *
from quex.core_engine.state_machine.core import StateMachine
from quex.core_engine.regular_expression.auxiliary import __snap_until, \
                                                          __debug_entry, \
                                                          __debug_exit, \
                                                          __debug_print

import quex.core_engine.utf8                                  as utf8
import quex.core_engine.regular_expression.character_set_expression   as character_set_expression
import quex.core_engine.regular_expression.snap_backslashed_character as snap_backslashed_character
import quex.core_engine.regular_expression.snap_character_string      as snap_character_string
import quex.core_engine.state_machine.sequentialize           as sequentialize
import quex.core_engine.state_machine.parallelize             as parallelize
import quex.core_engine.state_machine.repeat                  as repeat
import quex.core_engine.state_machine.setup_post_context      as setup_post_context
import quex.core_engine.state_machine.setup_pre_context       as setup_pre_context
import quex.core_engine.state_machine.setup_border_conditions as setup_border_conditions
import quex.core_engine.state_machine.nfa_to_dfa              as nfa_to_dfa
import quex.core_engine.state_machine.hopcroft_minimization   as hopcroft


CONTROL_CHARACTERS = [ "+", "*", "\"", "/", "(", ")", "{", "}", "|", "[", "]", "$"] 

def __clean_and_validate(sm, BufferLimitCode, AllowNothingIsFineF):
    """This function is to be used by the outer shell to the user. It ensures that 
       the state machine which is returned is conform to some assumptions.

       BLC == -1: means that there is no buffer limit code.
    """

    # (*) The buffer limit code has to appear absolutely nowhere!
    if BufferLimitCode != -1:
        __delete_BLC_except_at_end_of_post_context(sm, BufferLimitCode)

    # (*) Delete transitions that make practically no sense
    __delete_transitions_on_code_points_below_zero(sm)


    # (*) 'Nothing is fine' is not a pattern that we can accept. See the discussion
    #     in the module "quex.core_engine.generator.core.py"
    if sm.get_init_state().core().is_acceptance() and AllowNothingIsFineF == False: 
        raise RegularExpressionException(
                "Pattern results in a 'nothing is acceptable' state machine.\n" + \
                "This means, that no step forward in the input still sets the analyzer\n" + \
                "into an acceptance state. Thus, as soon as no other input matches\n" + \
                "the analyzer ends up in an infinite loop.")

    # (*) double check for orphan states
    orphan_state_list = sm.get_orphaned_state_index_list()
    if orphan_state_list != []:
        error_msg("Orphaned state(s) detected in regular expression (optimization lack).\n" + \
                  "Please, log a defect at the projects website quex.sourceforge.net.\n"    + \
                  "Orphan state(s) = " + repr(orphan_state_list)                       + "\n", 
                  fh, DontExitF=True)

    # (*) It is essential that state machines defined as patterns do not 
    #     have origins.
    if sm.has_origins():
        error_msg("Regular expression parsing resulted in state machine with origins.\n" + \
                  "Please, log a defect at the projects website quex.sourceforge.net.\n", fh)
        
    return sm

def __delete_BLC_except_at_end_of_post_context(sm, BLC):
    """The buffer limit code is something that **needs** to cause a drop out.
       In the drop out handling, the buffer is reloaded.
    """
    for state in sm.states.values():
        for target_state_index, trigger_set in state.transitions().get_map().items():
            if trigger_set.contains(BLC):
                trigger_set.cut_interval(Interval(BLC, BLC+1))

def __delete_transitions_on_code_points_below_zero(sm):
    """Unicode does define all code points >= 0. Thus there can be no code points
       below zero as it might result from some number set operations.
    """
    for state in sm.states.values():
        for target_state_index, trigger_set in state.transitions().get_map().items():
            if trigger_set.minimum() < 0:
                # NOTE: '0' is the end, meaning that it has is not part of the interval to be cut.
                trigger_set.cut_interval(Interval(-sys.maxint, 0))

def do(UTF8_String_or_Stream, PatternDict, BufferLimitCode,
       DOS_CarriageReturnNewlineF=False, AllowNothingIsFineF=False):


    if type(UTF8_String_or_Stream) == str: stream = StringIO.StringIO(UTF8_String_or_Stream)
    else:                                  stream = UTF8_String_or_Stream    

    if PatternDict == None: PatternDict = {}

    initial_position = stream.tell()

    # -- check for the begin of line condition (BOL)
    if stream.read(1) == '^': begin_of_line_f = True
    else:                     stream.seek(-1, 1); begin_of_line_f = False
    
    # -- MAIN: transform the pattern into a state machine
    sm = snap_conditional_expression(stream, PatternDict)
    if sm == None: 
        stream.seek(initial_position)
        return None
    
    # -- check for end of line condition (EOL)
    if stream.read(1) == '$': end_of_line_f = True
    else:                     stream.seek(-1, 1); end_of_line_f = False

    # -- set begin of line/end of line conditions
    if begin_of_line_f or end_of_line_f: 
        sm = setup_border_conditions.do(sm, begin_of_line_f, end_of_line_f,
                                        DOS_CarriageReturnNewlineF)
        sm = __beautify(sm)

    return __clean_and_validate(sm, BufferLimitCode, AllowNothingIsFineF)

def snap_conditional_expression(stream, PatternDict):
    """conditional expression: expression
                               expression / expression                 = post conditioned expression
                               expression / expression /               = pre conditioned expression
                               expression / expression / expression    = pre and post conditioned expression
       TODO: <- ($8592) for pre-conditions
             -> ($8594) for post-conditions

    """                     
    __debug_entry("conditional expression", stream)    

    # -- expression
    pattern_0 = snap_expression(stream, PatternDict) 
    if pattern_0 == None: return __debug_exit(None, stream)
    
    # -- '/'
    if stream.read(1) != '/': 
        # (1) expression without pre and post condition
        stream.seek(-1, 1)
        # pattern_0 is already beautified by 'snap_expression()'
        result = __construct(pattern_0)
        return __debug_exit(result, stream)
        
    # -- expression
    pattern_1 = snap_expression(stream, PatternDict) 
    if pattern_1 == None: return __debug_exit(pattern_0, stream)
    
    # -- '/'
    if stream.read(1) != '/': 
        # (2) expression with only a post condition
        stream.seek(-1, 1)
        #     NOTE: setup_post_context() marks state origins!
        result = __construct(pattern_0, post_context=pattern_1)
        return __debug_exit(result, stream)

    # -- expression
    pattern_2 = snap_expression(stream, PatternDict) 
    if pattern_2 == None: 
        # (3) expression with only a pre condition
        #     NOTE: setup_pre_context() marks the state origins!
        result = __construct(pattern_1, pre_context=pattern_0)
        return __debug_exit(result, stream)

    # (4) expression with post and pre-condition
    result = __construct(pattern_1, pre_context=pattern_0, post_context=pattern_2)
    return __debug_exit(result, stream)

def snap_expression(stream, PatternDict):
    """expression:  term
                    term | expression
    """              
    __debug_entry("expression", stream)    
    # -- term
    result = snap_term(stream, PatternDict) 
    if result == None: 
        return __debug_exit(None, stream)

    # -- optional '|'
    if stream.read(1) != '|': 
        stream.seek(-1, 1)
        return __debug_exit(result, stream)
    
    position_1 = stream.tell()
    __debug_print("'|' (in expression)")

    # -- expression
    result_2 = snap_expression(stream, PatternDict) 
    __debug_print("expression(in expression):",  result_2)
    if result_2 == None:
        stream.seek(position_1) 
        return __debug_exit(result, stream)

    result = parallelize.do([result, result_2])    
    return __debug_exit(__beautify(result), stream)
        
def snap_term(stream, PatternDict):
    """term:  primary
              primary term 
    """
    __debug_entry("term", stream)    

    # -- primary
    result = snap_primary(stream, PatternDict) 
    __debug_print("##primary(in term):", result)
    if result == None: return __debug_exit(None, stream)
    position_1 = stream.tell()

    # -- optional 'term' 
    result_2 = snap_term(stream, PatternDict) 
    __debug_print("##term(in term):",  result_2)
    if result_2 == None: 
        stream.seek(position_1)
        return __debug_exit(result, stream)
    
    result = sequentialize.do([result, result_2], 
                              MountToFirstStateMachineF=True, 
                              CloneRemainingStateMachinesF=False)    

    return __debug_exit(__beautify(result), stream)
        
def snap_primary(stream, PatternDict):
    """primary:  " non_double_quote *  "              = character string
                 [ non_rect_bracket_close ]           = set of characters
                 { identifier }                       = pattern replacement
                 ( expression )
                 non_control_character+               = lonely characters
                 primary repetition_cmd
    """
    __debug_entry("primary", stream)    
    x = stream.read(1)
    if x == "": return __debug_exit(None, stream)

    def eat_this(supposed_first_char, the_string):
        if len(the_string) < 1 or the_string[0] != supposed_first_char:
            raise RegularExpressionException("missing '%s'" % supposed_first_char + "\n" + \
                                             "remaining string = '%s'" % the_string) 
        return the_string[1:]    

    # -- 'primary' primary
    if   x == "\"": result = snap_character_string.do(stream)
    elif x == "[":  
        stream.seek(-1, 1); 
        result = character_set_expression.do(stream)
    elif x == "{":  result = snap_replacement(stream, PatternDict)
    elif x == ".":  result = create_ALL_BUT_NEWLINE_state_machine()
    elif x == "(": 
        __start_position = stream.tell()
        result = snap_expression(stream, PatternDict)
        if stream.read(1) != ")": 
            stream.seek(-1, 1)
            raise RegularExpressionException("missing closing ')' after expression. found '%s'" % stream.read())

        if result == None:
            __expression_length = stream.tell() - __start_position
            stream.seek(__start_position)
            raise RegularExpressionException("expression in brackets has invalid syntax '%s'" % \
                                             stream.read(__expression_length))

    elif x.isspace():
        # a lonestanding space ends the regular expression
        stream.seek(-1, 1)
        return __debug_exit(None, stream)

    elif x in ["*", "+", "?"]:
        raise RegularExpressionException("lonely operator '%s' without expression proceeding." % x) 

    elif x not in CONTROL_CHARACTERS:
        # NOTE: The '\' is not inside the control characters---for a reason.
        #       It is used to define for example character codes using '\x' etc.
        stream.seek(-1, 1)
        result = snap_non_control_characters(stream)

    else:
        # NOTE: This includes the '$' sign which means 'end of line'
        #       because the '$' sign is in CONTROL_CHARACTERS, but is not checked
        #       against. Thus, it it good to leave here on '$' because the
        #       '$' sign is handled on the very top level.
        # this is not a valid primary
        stream.seek(-1, 1)
        return __debug_exit(None, stream)

    # -- optional repetition command? 
    result_repeated = __snap_repetition_range(result, stream) 
    ## print "##imr:", result.get_string(NormalizeF=False)
    if result_repeated != None: result = result_repeated
    return __debug_exit(__beautify(result), stream)
    
def snap_non_control_characters(stream):
    """Snaps any 'non_control_character' using UTF8 encoding from the given string. Note, that 
       in UTF8 a character may consist of more than one byte. Creates a state machine 
       that contains solely one trigger for each character to a acceptance state.

       This function **concatinates** incoming characters, but **repetition** has preceedence
       over concatination, so it checks after each character whether it is followed by
       a repetition ('*', '+', '?', '{..}'). In such a case, the repetition of the character
       is appended.
    """
    __debug_entry("non-control characters", stream)

    result      = StateMachine()
    state_index = result.init_state_index
    # (*) read first character
    position  = stream.tell()
    char_code = utf8.__read_one_utf8_code_from_stream(stream)
    while char_code != 0xFF:
        # (1) check against occurence of control characters
        #     this needs to come **before** the backslashed character interpretation.
        #     NOTE: A backslashed character can be a whitespace (for example '\n'). 
        #     (check against 0xFF to avoid overflow in function 'chr()') 
        if char_code < 0xFF \
           and (chr(char_code) in CONTROL_CHARACTERS or chr(char_code).isspace()):
               stream.seek(-1, 1) 
               break 

        # (2) treat backslashed characters
        if char_code == ord('\\'):
            stream.seek(-1, 1)
            trigger_set = character_set_expression.snap_property_set(stream)
            if trigger_set == None:
                stream.seek(1, 1)  # snap_property_set() leaves tream right before '\\'
                char_code = snap_backslashed_character.do(stream)
                if char_code == None:
                    raise RegularExpressionException("Backslash followed by unrecognized character code.")
                trigger_set = char_code
        else:
            trigger_set = char_code

        # (3) read next character
        position       = stream.tell()
        next_char_code = utf8.__read_one_utf8_code_from_stream(stream)
        #    -- check for repetition (repetition has preceedence over concatination)
        if next_char_code in [ord("+"), ord("*"), ord("?"), ord("{")]:
            # (*) create state machine that consist of a single transition 
            tmp = StateMachine()
            tmp.add_transition(tmp.init_state_index, trigger_set, AcceptanceF=True)
            # -- repeat the single character state machine
            stream.seek(position)
            tmp_repeated = __snap_repetition_range(tmp, stream) 
            # -- append it to the result (last state must be set to acceptance for concatenation)
            result.states[state_index].set_acceptance()
            result = sequentialize.do([result, tmp_repeated], MountToFirstStateMachineF=True)
            # as soon as there is repetition there might be more than one acceptance
            # state and thus simple concatination via 'add_transition' fails.
            # let us return and check treat the remaining chars
            # at the next call to this function.
            return __debug_exit(result, stream)

        else:
            # (*) add new transition from current state to a new state triggering
            #     on the given character.
            state_index = result.add_transition(state_index, trigger_set)

        char_code = next_char_code

    # last character in the chain triggers an 'acceptance state'
    result.states[state_index].set_acceptance()
        
    return __debug_exit(result, stream)
    
def snap_replacement(stream, PatternDict):
    """Snaps a predefined pattern from the input string and returns the resulting
       state machine.
    """ 
    skip_whitespace(stream)
    pattern_name = read_identifier(stream)  
    if pattern_name == "":
        raise RegularExpressionException("Pattern replacement expression misses identifier after '{'.")
    skip_whitespace(stream)

    letter = stream.read(1)
    if letter != "}":
        raise RegularExpressionException("Pattern replacement expression misses closing '}' after '%s'." \
                                         % pattern_name)

    if PatternDict.has_key(pattern_name) == False:
        raise RegularExpressionException("Pattern of name '%s' was not defined in any preceding 'define { }' section" \
                                         % pattern_name)

    state_machine = PatternDict[pattern_name].state_machine
    # It is essential that state machines defined as patterns do not 
    # have origins. Otherwise, the optimization of patterns that
    # contain pattern replacements might get confused and can
    # not find all optimizations.
    assert state_machine.has_origins() == False
        
    return state_machine

def __snap_repetition_range(the_state_machine, stream):    
    """Snaps a string that represents a repetition range. The following 
       syntaxes are supported:
           '?'      one or none repetition
           '+'      one or arbitrary repetition
           '*'      arbitrary repetition (even zero)
           '{n}'    exactly 'n' repetitions
           '{m,n}'  from 'm' to 'n' repetitions
           '{n,}'   arbitrary, but at least 'n' repetitions
    """       
    assert the_state_machine.__class__.__name__ == "StateMachine", \
           "received object of type '%s'" % the_state_machine.__class__.__name__ + "\n" + \
           repr(the_state_machine)

    position_0 = stream.tell()
    x = stream.read(1)
    if   x == "+": result = repeat.do(the_state_machine, 1)
    elif x == "*": result = repeat.do(the_state_machine)
    elif x == "?": result = repeat.do(the_state_machine, 0, 1)
    elif x == "{":
        repetition_range_str = __snap_until(stream, "}")
        if len(repetition_range_str) and not repetition_range_str[0].isdigit():
            # no repetition range, so everything remains as it is
            stream.seek(position_0)
            return the_state_machine
            
        try:
            if repetition_range_str.find(",") == -1:
                # no ',' thus "match exactly a certain number": 
                # e.g. {4} = match exactly four repetitions
                number = int(repetition_range_str)
                result = repeat.do(the_state_machine, number, number)
                return result
            # a range of numbers is given       
            fields = repetition_range_str.split(",")
            fields = map(lambda x: x.strip(), fields)

            number_1 = int(fields[0].strip())
            if fields[1] == "": number_2 = -1                      # e.g. {2,}
            else:               number_2 = int(fields[1].strip())  # e.g. {2,5}  
            # produce repeated state machine 
            result = repeat.do(the_state_machine, number_1, number_2)
            return result
        except:
            raise RegularExpressionException("error while parsing repetition range expression '%s'" \
                                             % repetition_range_str)
    else:
        # no repetition range, so everything remains as it is
        stream.seek(position_0)
        return the_state_machine
    
    return result

def __set_end_of_line_post_context(sm, EndOfFileCode=0):
    """Appends a post condition to the state machine to handle the end of line
       statement. This consists in translating 'EndOfLine' into a state machine
       with 'Newline' or 'EndOfFile'. Thus, when one of both follows the current
       character is at the end of a line.

       If you want to use a different code for end of file, specify it via the
       first argument 'EndOfFile'.

       NOTE: This is fundamentally different from beginning of line (BOL). BOL
             can be achieved by letting the state machine raise the corresponding
             flag. End of line post conditions rely on external algorithms for
             mounting a post-condition.
    """
    post_context_sm = StateMachine()
    post_context_sm.add_transition(post_context_sm.init_state_index, ord('\n'), AcceptanceF=True)
    post_context_sm.add_transition(post_context_sm.init_state_index, EndOfFileCode, AcceptanceF=True)

    result = setup_post_context.do(sm, post_context_sm)

    return result

def __beautify(the_state_machine):
    ## assert the_state_machine.get_orphaned_state_index_list() == [], \
    ##       "before conversion to DFA: orphaned states " + repr(the_state_machine)
    result = nfa_to_dfa.do(the_state_machine)
    ## assert the_state_machine.get_orphaned_state_index_list() == [], \
    ##       "after conversion to DFA: orphaned states " + repr(the_state_machine)
    result = hopcroft.do(result)#, CreateNewStateMachineF=False)
    ## assert the_state_machine.get_orphaned_state_index_list() == [], \
    ##       "after hopcroft minimization: orphaned states " + repr(the_state_machine)

    return result

def __construct(core_sm, pre_context=None, post_context=None):

    if   pre_context == None and post_context == None:
        result = core_sm
        # -- can't get more beautiful ...
    
    elif pre_context == None and post_context != None:
        result = setup_post_context.do(core_sm, post_context)
        result = __beautify(result)

    elif pre_context != None and post_context == None:
        result = setup_pre_context.do(core_sm, pre_context)
        result = __beautify(result)

    elif pre_context != None and post_context != None:
        # NOTE: pre-condition needs to be setup **after** post condition, because
        #       post condition deletes all origins!
        #       (is this still so? 07y7m6d fschaef)
        result = setup_post_context.do(core_sm, post_context)
        result = setup_pre_context.do(result, pre_context)
        result = __beautify(result)

    return result
  
def create_ALL_BUT_NEWLINE_state_machine():
    result = StateMachine()
    # NOTE: Buffer control characters are supposed to be filtered out by the code
    #       generator.
    trigger_set = NumberSet(Interval(ord("\n")).inverse()) 

    result.add_transition(result.init_state_index, trigger_set, AcceptanceF=True) 
    return result
    
