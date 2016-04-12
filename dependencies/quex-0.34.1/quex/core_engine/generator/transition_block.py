import sys
import quex.core_engine.generator.transition as transition
from   quex.input.setup import setup as Setup
LanguageDB = Setup.language_db

__DEBUG_CHECK_ACTIVE_F = False # Use this flag to double check that intervals are adjacent

class __info:
    def __init__(self, StateIdx, IsInitStateF, DSM):
        assert DSM == None or DSM.__class__.__name__ == "StateMachineDecorator"

        self.state_index        = StateIdx
        self.is_init_state_f    = IsInitStateF

        self.dsm = DSM

def do(TriggerMap, StateIdx, InitStateF, DSM):
    assert type(TriggerMap) == list
    assert DSM == None or DSM.__class__.__name__ == "StateMachineDecorator"
    # If a state has no transitions, no new input needs to be eaten => no reload.
    #
    # NOTE: The only case where the buffer reload is not required are empty states,
    #       AND states during backward input position detection!
    #       Empty states do not exist any longer, the backward input position is
    #       essential though for pseudo ambiguous post contexts.
    assert TriggerMap != [] # states with empty trigger maps are 'dead end states'. those
    #                       # are not to be coded at this place.

    info = __info(StateIdx=StateIdx, IsInitStateF=InitStateF, DSM=DSM)

    if len(TriggerMap) > 1:
        return __get_code(TriggerMap, info) + "\n"
    else:
        # We can actually be sure, that the Buffer Limit Code is filtered
        # out, since this is the task of the regular expression parser.
        # In case of backward lexing in pseudo-ambiguous post conditions,
        # it makes absolutely sense that there is only one interval that
        # covers all characters (see the discussion there).
        assert TriggerMap[0][0].begin == -sys.maxint
        assert TriggerMap[0][0].end   == sys.maxint
        return  "    " + transition.do(StateIdx, TriggerMap[0][0], TriggerMap[0][1], DSM) + "\n"

def __get_code(TriggerMap, info):
    """Creates code for state transitions from this state. This function is very
       similar to the function creating code for a 'NumberSet' condition 
       (see 'interval_handling').
    
       Writes code that does a mapping according to 'binary search' by
       means of if-else-blocks.
    """
    TriggerSetN = len(TriggerMap)

    if TriggerSetN > 1 and __DEBUG_CHECK_ACTIVE_F:
        # -- check that the trigger map consist of sorted adjacent intervals 
        #    This assumption is critical because it is assumed that for any isolated
        #    interval the bordering intervals have bracketed the remaining cases!
        previous_interval = TriggerMap[0][0] 
        for trigger_interval, target_state_index in TriggerMap[1:]:
            assert trigger_interval.begin == previous_interval.end, \
                   "non-adjacent intervals in TriggerMap\n" + \
                   "TriggerMap = " + repr(TriggerMap)
            assert trigger_interval.end > previous_interval.begin, \
                   "unsorted intervals in TriggerMap\n" + \
                   "TriggerMap = " + repr(TriggerMap)
            previous_interval = deepcopy(trigger_interval)

    #________________________________________________________________________________
    txt = "    "

    if TriggerSetN == 1 :
        # (*) Only one interval 
        #     (all boundaring cases must have been dealt with already => case is clear)
        #     If the input falls into this interval the target trigger is identified!
        txt += __create_transition_code(TriggerMap[0], info)
        
    else:    
        simple_txt = __try_very_simplest_case(TriggerMap, info)
        if simple_txt != None: 
            txt += simple_txt
        else:
            # two or more intervals => cut in the middle
            MiddleTrigger_Idx = int(TriggerSetN / 2)
            middle = TriggerMap[MiddleTrigger_Idx]

            # input < 0 is impossible, since unicode codepoints start at 0!
            if middle[0].begin == 0: txt += __get_code(TriggerMap[MiddleTrigger_Idx:], info) 
            elif TriggerSetN == 2:   txt += __bracket_two_intervals(TriggerMap, info) 
            elif TriggerSetN == 3:   txt += __bracket_three_intervals(TriggerMap, info)
            else:                    txt += __bracket_normally(MiddleTrigger_Idx, TriggerMap, info)
        

    # (*) indent by four spaces (nested blocks are correctly indented)
    #     delete the last newline, to prevent additional indentation
    if txt[-1] == "\n": txt = txt[:-1]
    txt = txt.replace("\n", "\n    ") + "\n"
    return txt 

def __create_transition_code(TriggerMapEntry, info, IndentF=False):
    """Creates the transition code to a given target based on the information in
       the trigger map entry.
    """
    interval           = TriggerMapEntry[0]
    target_state_index = TriggerMapEntry[1]       
    #  target state != None, then the machine is still eating
    #                        => transition to subsequent state.
    #
    #  target state == None, drop into a terminal state (defined by origins).
    #
    #  for details about $transition, see the __transition() function of the
    #  respective language module.
    #
    txt =  "    " + transition.do(info.state_index, interval, target_state_index, info.dsm)
    if interval != None:
        txt += "    " + LanguageDB["$comment"](interval.get_utf8_string()) + "\n"
    else:
        txt += "\n"

    if IndentF: 
        txt = txt[:-1].replace("\n", "\n    ") + "\n" # don't replace last '\n'
    return txt

def __try_very_simplest_case(TriggerMap, info):
    """Assume the following setup:

           if( input == Char1 ) goto X;
           if( input == Char2 ) goto X;
               ...
           if( input == CharN ) goto X;

       If the input is equally distributed over the characters 1 to N then the
       average number of comparisons for N = 3 will be 2,333. For N = 4, the 
       everage number of comparisons will be 2,75. Binary bracketing requires
       ld(N), so for N = 4 the number of comparisons is 2. Thus until N = 3
       it is advantegous to compare step by step. Also, for N = 1 a simple 
       comparison is, most likely, more efficient that an 'or' operation over
       a list of length '1'. 
       
       This function is trying to identify the case where there are only two or
       three characters that trigger to the same target state. 
       
       RETURNS:  'None' if the very simple implementation does not make sense.
                 A string if it could be implemented that way
    """
    # return None
    character_list            = []
    common_target_state_index = -1
    for trigger in TriggerMap:
        interval           = trigger[0]
        target_state_index = trigger[1]

        if target_state_index == None: continue
        assert target_state_index != -1

        # All must have the same target state
        if common_target_state_index == -1:
            common_target_state_index = target_state_index
        elif common_target_state_index != target_state_index: 
            return None

        # Because of memory reasons, it is not wise to try to extend sys.maxint number
        # of characters. Since, we do not allow for more than three characters, let's
        # do a little sanity pre-check:
        if interval.size() > 3: return None
        character_list.extend(range(interval.begin, interval.end))

        # More than three characters does not make sense
        if len(character_list) > 3: return None

    if len(character_list) < 2: return None
    assert common_target_state_index != -1

    txt = LanguageDB["$if in-set"](character_list)    
    # TriggerInfo = [None, TargetStateIndex] because the interval does not matter.
    txt += __create_transition_code([None, common_target_state_index], info, IndentF=True) 
    txt += LanguageDB["$endif-else"]
    txt += __create_transition_code([None, None], info, IndentF=True)
    txt += LanguageDB["$end-else"]

    return txt

        
def __bracket_two_intervals(TriggerMap, info):
    assert len(TriggerMap) == 2

    first  = TriggerMap[0]
    second = TriggerMap[1]

    # If the first interval causes a 'drop out' then make it the second.
    ## If the second interval is a 'drop out' the 'goto drop out' can be spared,
    ## since it lands there anyway.
    ## if second[0] < 0: # target state index < 0 ==> drop out
    ##    tmp = first; first = second; second = tmp

    # find interval of size '1'
    first_interval  = first[0]
    second_interval = second[0]

    # We only need one comparison at the border between the two intervals
    if   first_interval.size() == 1:  txt = LanguageDB["$if =="](repr(first_interval.begin))
    elif second_interval.size() == 1: txt = LanguageDB["$if !="](repr(second_interval.begin))
    else:                             txt = LanguageDB["$if <"](repr(second_interval.begin))

    txt += __create_transition_code(first, info, IndentF=True) 
    txt += LanguageDB["$endif-else"]
    txt += __create_transition_code(second, info, IndentF=True)
    txt += LanguageDB["$end-else"]

    return txt

def __bracket_three_intervals(TriggerMap, info):
    assert len(TriggerMap) == 3

    # does one interval have the size '1'?
    size_one_map = [False, False, False]   # size_on_map[i] == True if interval 'i' has size '1'
    for i in range(len(TriggerMap)):
        interval = TriggerMap[i][0]
        if interval.size() == 1: size_one_map[i] = True

    target_state_0 = TriggerMap[0][1]
    target_state_2 = TriggerMap[2][1]
    if target_state_0 == target_state_2:
        if TriggerMap[1][0].size() == 1:
            # (1) Special Trick I only holds for one single case:
            #     -- the interval in the middle has size 1
            #     -- the outer two intervals trigger to the same target state
            #     if inner character is matched: goto its target
            #     else:                          goto alternative target
            txt  = LanguageDB["$if =="](repr(TriggerMap[1][0].begin))
        else:
            # (2) Special Trick II only holds for:
            #     -- the outer two intervals trigger to the same target state
            #     if character in inner interval: goto its target
            #     else:                           goto alternative target
            txt  = LanguageDB["$if in-interval"](TriggerMap[1][0])
        txt += __create_transition_code(TriggerMap[1], info, IndentF=True) 
        txt += LanguageDB["$endif-else"]
        # TODO: Add somehow a mechanism to report that here the intervals 0 **and** 1 are triggered
        #       (only for the comments in the generated code)
        txt += __create_transition_code(TriggerMap[0], info, IndentF=True)
        txt += LanguageDB["$end-else"]
        return txt

    # (*) Non special case --> bracket normally
    return __bracket_normally(1, TriggerMap, info)

def __bracket_normally(MiddleTrigger_Idx, TriggerMap, info):

    middle = TriggerMap[MiddleTrigger_Idx]
    assert middle[0].begin >= 0, \
           "code generation: error cannot split intervals at negative code points."

    txt =  LanguageDB["$if <"](repr(middle[0].begin))
    txt += __get_code(TriggerMap[:MiddleTrigger_Idx], info)
    txt += LanguageDB["$endif-else"]
    txt += __get_code(TriggerMap[MiddleTrigger_Idx:], info)
    txt += LanguageDB["$end-else"]

    return txt





