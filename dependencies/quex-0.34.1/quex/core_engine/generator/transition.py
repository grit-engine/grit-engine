import quex.core_engine.generator.acceptance_info as acceptance_info
from   quex.input.setup import setup as Setup


def __goto_distinct_terminal(Origin):
    assert Origin.is_acceptance()
    LanguageDB = Setup.language_db
    txt = ""
    if Origin.post_context_id() == -1:
        # The seek for the end of the core pattern is part of the 'normal' terminal
        # if the terminal 'is' a post conditioned pattern acceptance.
        txt += LanguageDB["$input/increment"] + "\n"
    txt += LanguageDB["$goto"]("$terminal-direct", Origin.state_machine_id)
    return txt


def do(CurrentStateIdx, TriggerInterval, TargetStateIdx, DSM):
    """
        TargetStateIdx: != None: Index of the state to which 'goto' has to go.
                        == None: Drop Out. Goto a terminal state.
        DSM == None: We are not concerned with the whole state machine and just want to
                     create a nice binary-bracketing transition (e.g. for range skippers).
    """
    LanguageDB = Setup.language_db
    assert DSM == None            or DSM.__class__.__name__ == "StateMachineDecorator"
    assert TargetStateIdx == None or TargetStateIdx >= 0
    assert TriggerInterval.__class__.__name__ == "Interval" or TriggerInterval == None

    if DSM == None or not DSM.dead_end_state_db().has_key(TargetStateIdx):
        # (1) The target state is not mentioned to be a state void of further transitions.
        if TargetStateIdx != None:   
            # THE very normal transition to another state
            return LanguageDB["$goto"]("$entry", TargetStateIdx)
        else:
            # NOTE: The normal drop out contains a check against the buffer limit code. This
            #       check can be avoided, if one is sure that the current interval does not contain
            #       a buffer limit code.
            blc = Setup.buffer_limit_code
            if type(blc) != int:
                if len(blc) > 2 and blc[:2] == "0x": blc = int(blc, 16)
                else:                                blc = int(blc)
            if TriggerInterval == None or TriggerInterval.contains(blc):
                return LanguageDB["$goto"]("$drop-out", CurrentStateIdx)
            else:
                return LanguageDB["$goto"]("$drop-out-direct", CurrentStateIdx)
    else:
        # (2) The TargetStateIdx is mentioned to be a dead-end-state! That means, that there is
        #     actually no 'transition body' in that state and it transits directly to a terminal.
        dead_end_target_state = DSM.dead_end_state_db()[TargetStateIdx]
        assert dead_end_target_state.is_acceptance(), \
               "NON-ACCEPTANCE dead end detected during code generation!\n" + \
               "A dead end that is not deleted must be an ACCEPTANCE dead end. See\n" + \
               "state_machine.dead_end_analysis.py and generator.state_machine_coder.py.\n" + \
               "If this is not the case, then something serious went wrong. A transition\n" + \
               "to a non-acceptance dead end is to be translated into a drop-out."

        if dead_end_target_state.origins().contains_any_pre_context_dependency(): 
            # Backward lexing (pre-condition or backward input position detection) cannot
            # depend on pre-conditions, since it is not part of the 'main' lexical analyser
            # process.
            assert DSM.mode() == "ForwardLexing"
            return LanguageDB["$goto"]("$entry", TargetStateIdx)   # router to terminal

        elif DSM.mode() == "ForwardLexing":
            winner_origin = dead_end_target_state.origins().find_first_acceptance_origin()
            assert type(winner_origin) != type(None)               # see first assert in this block

            terminal_id = winner_origin.state_machine_id

            # During forward lexing (main lexer process) there are dedicated terminal states.
            return __goto_distinct_terminal(winner_origin)

        elif DSM.mode() == "BackwardLexing":
            # When checking a pre-condition no dedicated terminal exists. However, when
            # we check for pre-conditions, a pre-condition flag needs to be set.
            return acceptance_info.backward_lexing(dead_end_target_state)

        elif DSM.mode() == "BackwardInputPositionDetection":
            # When searching backwards for the end of the core pattern, and one reaches
            # a dead end state, then no position needs to be stored extra since it was
            # stored at the entry of the state.
            txt  = LanguageDB["$input/decrement"] + "\n"
            txt += acceptance_info.backward_lexing_find_core_pattern(dead_end_target_state)

            txt += LanguageDB["$goto"]("$terminal-general", True)   # general terminal
            return txt
        else:
            assert False, "Impossible lexing mode: '%s'" % DSM.mode()
    
    

def do_dead_end_router(State, StateIdx, BackwardLexingF):
    # DeadEndType == -1:
    #    States, that do not contain any acceptance transit to the 'General Terminal'
    #    The do not have to be coded. Instead the 'jump' must be redirected immediately
    #    to the general terminal.
    # DeadEndType == some integer:
    #    States, where the acceptance is clear must be redirected to the correspondent
    #    terminal given by the integer.
    # DeadEndType == None:
    #    States, where the acceptance depends on the run-time pre-conditions being fulfilled
    #    or not. They are the only once, that are 'implemented' as routers, that map to
    #    a terminal correspondent the pre-conditions.
    assert State.is_acceptance()

    if State.origins().contains_any_pre_context_dependency() == False: 
        return ""

    txt = acceptance_info.get_acceptance_detector(State.origins().get_list(), 
                                                  __goto_distinct_terminal)
            
    # -- double check for consistency
    assert txt != "", "Acceptance state without acceptance origins!"        

    return txt
