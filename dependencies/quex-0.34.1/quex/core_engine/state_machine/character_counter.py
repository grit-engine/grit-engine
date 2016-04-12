from quex.core_engine.interval_handling import Interval, NumberSet

__distance_db = {}
def get_newline_n(state_machine):   
    """
       Counts the number of newlines that appear until the acceptance state. 
       The part of the post condition is omitted. 

       RETURNS:  0      if statemachine / pattern does not contain the newline
                        character at all.
                 N > 0  number of newlines that are **always** required in order to
                        reach an acceptance state.
                 -1     the number of newlines cannot be determined, because of 
                        recursion or because there are different pathes to acceptance
                        with different numbers of newlines occuring.

       NOTE: Only the core pattern is concerned---not the pre- or post-condition.
    """
    global __distance_db
    __distance_db.clear()
    return __dive(state_machine, state_machine.init_state_index, 0, [], CharacterToCount=ord('\n'))

def get_character_n(state_machine):
    """
       Counts the number of characters that appear until the acceptance state. 
       The part of the post condition is omitted. 

       RETURNS:  0      if statemachine / pattern does not contain the newline
                        character at all.
                 N > 0  number of newlines that are **always** required in order to
                        reach an acceptance state.
                 -1     the number of newlines cannot be determined, because of 
                        recursion or because there are different pathes to acceptance
                        with different numbers of newlines occuring.

       NOTE: Only the core pattern is concerned---not the pre- or post-condition.
    """
    global __distance_db
    __distance_db.clear()
    return __dive(state_machine, state_machine.init_state_index, 0, [], CharacterToCount=-1)

def contains_only_spaces(state_machine):
    """Determines wether there are only spaces on the way to the acceptance state.
    """
    for state in state_machine.states.values():
        target_state_list = state.transitions().get_target_state_index_list()
        # (1) if a pattern contains only ' ', then there is no place for more than
        #     one target state, since every state has only one trigger and one target state
        if len(target_state_list) > 1: return False

        # (2) does state exclusively trigger on ' '?
        #    (2a) does state trigger on ' '?
        all_trigger_set = state.transitions().get_trigger_set_union()
        if all_trigger_set.contains(ord(' ')) == False: return False
        #    (2b) does state trigger on nothing else? 
        if all_trigger_set.difference(NumberSet(ord(' '))).is_empty() == False: return False

    return True

def __recursion_contains_critical_character(state_machine, Path, TargetStateIdx, Character):
    """Path      = list of state indices
       Character = character code of concern.
                   -1 => any character.
       
       RETURNS:  True = the path contains the TargetStateIdx and on the path
                        there is the critical character.
                 False = the path does either not contain the TargetStateIdx,
                         i.e. there will be no recursion, or the recursion
                         path does not contain the critical character and 
                         therefore is not dangerous.

       NOTE: This function is required to judge wether a recursion occurs
             that effects the number of characters to be counted. If so,
             then the recursion signifies that the number of characters
             to be matched cannot be determined directly from the state machine.
             They have to be computed after a match has happend.
    """
    assert TargetStateIdx in Path 

    # -- recursion detected!
    #    did the critical character occur in the path?
    if Character == -1: return True

    occurence_idx = Path.index(TargetStateIdx)
    prev_idx      = TargetStateIdx
    for idx in Path[occurence_idx+1:] + [TargetStateIdx]:
        # does transition from prev_state to state contain newline?
        trigger_set = state_machine.states[prev_idx].transitions().get_trigger_set_to_target(idx)
        if trigger_set.contains(Character):
            return True                       # YES! recursion with critical character
        prev_idx = idx

    # -- no critical character in recursion --> OK, no problem
    # -- state has been already handled, no further treatment required
    return False

def __recursion_with_critical_character_ahead(state_machine, state, PassedStateList, Character):
    """Does any of the following target states close a recursion loop which contains the 
       character to be counted?
    """

    for follow_state_index in state.transitions().get_target_state_index_list():

        if follow_state_index not in PassedStateList: continue

        if __recursion_contains_critical_character(state_machine, PassedStateList, follow_state_index, Character):
            return True

    return False

def __dive(state_machine, state_index, character_n, passed_state_list, CharacterToCount):
    """Once the distance to the acceptance state is determined, we store it in a cache database.
       Note, that the distance is only stored after all possible pathes starting from the state
       have been investigated. Note also, that the distance to the acceptance state can be 
       '-1' meaning that there are multiple pathes of different length, i.e. it cannot be
       determined from the pattern how many characters appear in the lexeme that matches.
    """
    global __distance_db
    if __distance_db.has_key(state_index): 
        return __distance_db[state_index] + character_n

    result = ____dive(state_machine, state_index, character_n, passed_state_list, CharacterToCount)
    if state_index not in passed_state_list:
        __distance_db[state_index] = result - character_n

    return result


def ____dive(state_machine, state_index, character_n, passed_state_list, CharacterToCount):
    state = state_machine.states[state_index]

    new_passed_state_list = passed_state_list + [ state_index ]

    # -- recursion?
    if __recursion_with_critical_character_ahead(state_machine, state, new_passed_state_list, 
                                                 CharacterToCount):
        return -1

    # -- if no recursion is detected and the state is the end of a core exression
    #    of a post conditioned pattern, then this is it. No further investigation
    #    from this point on. The post condition state machine is not considered 
    #    for line number counting.
    if state.core().post_context_id() != -1L: return character_n

    # trigger_map[target_state_index] = set that triggers to target state index
    trigger_dict = state.transitions().get_map()
    if trigger_dict == {}: return character_n
        
    if state.is_acceptance():  prev_characters_found_n = character_n
    else:                      prev_characters_found_n = None

    for follow_state_index, trigger_set in trigger_dict.items():

        # -- do not follow recursive paths. note: this is only relevant for specified
        #    CharacterToCount != -1. Otherwise, recursions are broken up when detected ahead.
        if follow_state_index in new_passed_state_list: continue
            
        if CharacterToCount == -1:
            # (1.1) We are counting all characters, so we increment always.
            increment = 1
        elif not trigger_set.contains(CharacterToCount):
            # (2.1) The trigger set does not contain the character to be counted at all
            #       Thus the number of occurences is deterministic and **no increment occurence counter**.
            increment = 0
        elif trigger_set.has_only_this_element(CharacterToCount):
            # (2.2) The trigger set contains only the character to be counted.
            #       Thus the number of occurences is deterministic and **increment occurence counter**.
            increment = 1
        else:
            # (2.3) The trigger set contains the character to be counted and also others. This
            #       means that for the transition the number of occurences (zero or one) is not
            #       determined by the pattern. Thus the number of occurences not deterministic.
            return -1

        characters_found_n = __dive(state_machine, follow_state_index, character_n + increment, 
                                    new_passed_state_list, CharacterToCount)

        # -- if one path contains an undefined number of characters, then the whole pattern
        #    has an undefined number of characters.
        if characters_found_n == -1: 
            return -1

        # -- if one path contains a different number of characters than another path
        #    then the number of characters is undefined.
        if prev_characters_found_n != None and \
           prev_characters_found_n != characters_found_n: 
               return -1

        prev_characters_found_n = characters_found_n

    if prev_characters_found_n == None: return -1
    else:                               return prev_characters_found_n


