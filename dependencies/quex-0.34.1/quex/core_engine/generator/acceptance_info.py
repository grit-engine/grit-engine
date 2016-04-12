from quex.core_engine.generator.languages.core import __nice
from quex.input.setup import setup as Setup
LanguageDB = Setup.language_db

def do(State, StateIdx, SMD):
    assert State.__class__.__name__ == "State"
    assert SMD.__class__.__name__   == "StateMachineDecorator"
    
    mode = SMD.mode()
    if   mode == "ForwardLexing":                  return forward_lexing(State, StateIdx, SMD)
    elif mode == "BackwardLexing":                 return backward_lexing(State)
    elif mode == "BackwardInputPositionDetection": return backward_lexing_find_core_pattern(State)
    else:
        assert False, "This part of the code should never be reached"

def forward_lexing(State, StateIdx, SMD, ForceF=False):
    """Forward Lexing:

       (1) If a the end of a core pattern of a post contexted pattern is reached, then
           the current position needs to be stored in a dedicated register---ultimatively.

       (2) Final acceptance states can contain pre-conditions that have to be checked
           and their might me priorities.

           => use 'get_acceptance_detector()' in order to get a sequence of 'if-else'
              blocks that determine acceptance. 
    """
    assert SMD.__class__.__name__ == "StateMachineDecorator"
    SM = SMD.sm()

    OriginList = State.origins().get_list()

    txt = ""

    # (1) Set the post context registers (if appropriate)
    #     (also determine the list of acceptance origins)
    contains_acceptance_f = False
    for origin in OriginList: 
        if origin.is_end_of_post_contexted_core_pattern():
            assert origin.is_acceptance() == False
            # store current input position, to be restored when post condition really matches
            post_context_index = SMD.get_post_context_index(origin.state_machine_id)
            txt += "    " + LanguageDB["$comment"]("post context index '%s' == state machine '%s'" % \
                                                   (__nice(post_context_index), __nice(origin.state_machine_id)))
            txt += "    " + LanguageDB["$input/tell_position"]("post_context_start_position[%i]\n" % post_context_index)

        elif origin.is_acceptance():
            contains_acceptance_f = True

    # -- If there is no 'real' acceptance, then we're done
    if not contains_acceptance_f: 
        return txt 

    # -- If the current acceptance does not need to be stored, then do not do it
    if not ForceF and \
       do_subsequent_states_require_storage_of_last_acceptance_position(StateIdx, State, SM) == False: 
        return txt
   
    # (2) Create detector for normal and pre-conditioned acceptances
    def __on_detection_code(Origin):
        """Store the name of the winner pattern (last_acceptance) and the position
           where it has matched (use of $input/tell_position).
        """
        assert Origin.is_acceptance()
        info = LanguageDB["$set-last_acceptance"](__nice(Origin.state_machine_id))
        # NOTE: When a post conditioned pattern ends it does not store the input position.
        #       Rather, the acceptance position of the core pattern is considered in the
        #       terminal state.
        if Origin.post_context_id() == -1:
             info += LanguageDB["$input/tell_position"]("last_acceptance_input_position") + "\n"

        return info

    txt += get_acceptance_detector(OriginList, __on_detection_code)

    return txt

def backward_lexing(State):
    """Backward Lexing:
       -- Using an inverse state machine from 'real' current start position backwards
          until a drop out occurs.
       -- During backward lexing, there is no 'winner' so all origins that indicate
          acceptance need to be considered. They raise there flag 'pre-condition fulfilled'.
    """
    assert State.__class__.__name__ == "State"
    OriginList = State.origins().get_list()

    # There should be nothing, but unconditional acceptances or no-acceptance 
    # origins in the list of origins.
    inadmissible_origin_list = filter(lambda origin:
                                      origin.pre_context_begin_of_line_f() or
                                      origin.pre_context_id() != -1L or
                                      origin.post_context_id() != -1L,
                                      OriginList)
    assert inadmissible_origin_list == [], \
           "Inadmissible origins for inverse state machine."
    #___________________________________________________________________________________________

    txt = ""
    for origin in OriginList:
        if not origin.store_input_position_f(): continue
        assert origin.is_acceptance()
        variable = "pre_context_%s_fulfilled_f" % __nice(origin.state_machine_id)
        txt += "    " + LanguageDB["$assignment"](variable, 1)
    txt += "\n"    

    return txt

def backward_lexing_find_core_pattern(State):
    """Backward Lexing:
       -- (see above)
       -- for the search of the end of the core pattern, the acceptance position
          backwards must be stored. 
       -- There is only one pattern involved, so no determination of 'who won'
          is important.
    """
    assert State.__class__.__name__ == "State", \
           "Received %s as argument." % repr(State)

    OriginList = State.origins().get_list()

    # There should be nothing, but unconditional acceptances or no-acceptance 
    # origins in the list of origins.
    inadmissible_origin_list = filter(lambda origin:
                                      origin.pre_context_begin_of_line_f() or
                                      origin.pre_context_id() != -1L or
                                      origin.post_context_id() != -1L,
                                      OriginList)
    assert inadmissible_origin_list == [], \
           "Inadmissible origins for inverse state machine."
    #___________________________________________________________________________________________


    for origin in OriginList:
        if origin.store_input_position_f():
            assert origin.is_acceptance()
            return "    " + LanguageDB["$input/tell_position"]("end_of_core_pattern_position") + "\n"
    return "\n"

def get_acceptance_detector(OriginList, get_on_detection_code_fragment):
        
    LanguageDB = Setup.language_db

    def indent_this(Fragment):
        # do not replace the last '\n' with '\n    '
        return "    " + Fragment[:-1].replace("\n", "\n    ") + Fragment[-1]

    txt = ""
    first_if_statement_f = True
    OriginList.sort()
    for origin in OriginList:
        if not origin.is_acceptance(): continue

        info = get_on_detection_code_fragment(origin)

        if origin.pre_context_id() != -1L:
            if first_if_statement_f: txt += LanguageDB["$if pre-context"](origin.pre_context_id())
            else:                    txt += LanguageDB["$elseif pre-context"](origin.pre_context_id())
            txt += indent_this(info)
            txt += LanguageDB["$endif"]
        
        elif origin.pre_context_begin_of_line_f():
            if first_if_statement_f: txt += LanguageDB["$if begin-of-line"]
            else:                    txt += LanguageDB["$elseif begin-of-line"]
            txt += indent_this(info)
            txt += LanguageDB["$endif"] 
        
        else:
            if first_if_statement_f: 
                txt += info
            else:
                # if an 'if' statements preceeded, the acceptance needs to appear in an else block
                txt += LanguageDB["$else"] + "\n"; 
                txt += indent_this(info)
                txt += LanguageDB["$endif"]

            break  # no need for further pre-condition consideration

        first_if_statement_f = False

    # (*) write code for the unconditional acceptance states
    if txt == "": return ""
    else:         return "    " + txt[:-1].replace("\n", "\n    ") + txt[-1]

def do_subsequent_states_require_storage_of_last_acceptance_position(StateIdx, State, SM):
    """For the 'longest match' approach it is generally necessary to store the last
       pattern that has matched the current input stream. This means, that the
       current pattern index **and** the current input position need to be stored.
       Nothing of that is necessary, if one knows that a state does not have any
       'critical' follow-up states where the position is to be restored.
    """
    assert State.__class__.__name__ == "State"
    assert SM.__class__.__name__ == "StateMachine"
    assert State.is_acceptance()
    reachable_state_list = State.transitions().get_target_state_index_list()

    # If all directly following states are acceptance states then there is no
    # point in storing the last acceptance. It is overridden anyway at the 
    # entry of the next state. If one single state is non-acceptance then we
    # need to store the acceptance (return True).
    for state_index in reachable_state_list:
        if SM.states[state_index].is_acceptance() == False: return True
    return False



