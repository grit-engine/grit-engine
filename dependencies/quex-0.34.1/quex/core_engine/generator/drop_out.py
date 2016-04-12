from   quex.input.setup import setup as Setup
import quex.core_engine.generator.acceptance_info  as acceptance_info
LanguageDB = Setup.language_db

def do(state, StateIdx, SMD, InitStateF):
    """There are two reasons for drop out:
       
          (1) A buffer limit code has been reached.

          (2) The current character does not fit in the trigger map (regular drop out).
       
       Case (1) differs from case (2) in the fact that input == buffer limit code. 
       If a buffer limit code is reached, a buffer reload needs to be initiated. If
       the character drops over the edge, then a terminal state needs to be targeted.
    """
    assert SMD.__class__.__name__ == "StateMachineDecorator"

    TriggerMap = state.transitions().get_trigger_map()

    # -- drop out code (transition to no target state)
    txt = LanguageDB["$label-def"]("$drop-out", StateIdx)
    txt += "    " + LanguageDB["$if not BLC"]
    # -- if it's clear that it's not a buffer limit code, then jump directly
    txt += LanguageDB["$label-def"]("$drop-out-direct", StateIdx)
    txt += "        " + get_drop_out_goto_string(state, StateIdx, SMD.sm(), SMD.backward_lexing_f()) + "\n" 
    txt += "    " + LanguageDB["$endif"] + "\n"

    # -- in case of the init state, the end of file has to be checked.
    #    (there is no 'begin of file' action in a lexical analyzer when stepping backwards)
    if InitStateF and SMD.backward_lexing_f() == False:
        txt += "    " + LanguageDB["$if EOF"]
        comment = "NO CHECK 'last_acceptance != -1' --- first state can **never** be an acceptance state" 
        txt += "        " + LanguageDB["$comment"](comment) + "\n"
        txt += "        " + LanguageDB["$goto"]("$terminal-EOF") + "\n"
        txt += "    " + LanguageDB["$endif"]

    BufferReloadRequiredOnDropOutF = TriggerMap != [] and not SMD.backward_input_position_detection_f()
    if BufferReloadRequiredOnDropOutF:
        if SMD.backward_lexing_f():
            txt += __reload_backward(StateIdx)
        else:
            # In case that it cannot load anything, it still needs to know where to jump to.
            txt += "    " + acceptance_info.forward_lexing(state, StateIdx, SMD, ForceF=True)
            txt += __reload_forward(StateIdx)

    return txt + "\n"

def get_forward_load_procedure(StateIndex):
    txt  = '    QUEX_DEBUG_PRINT(&me->buffer, "FORWARD_BUFFER_RELOAD");\n'
    txt += "    if( QuexAnalyser_buffer_reload_forward(&me->buffer, &last_acceptance_input_position,\n"
    txt += "                                           post_context_start_position, PostContextStartPositionN) ) {\n"
    txt += "       " + LanguageDB["$goto"]("$input", StateIndex) + "\n"
    txt += "    " + LanguageDB["$endif"]                         + "\n"
    return txt

def __reload_forward(StateIndex): 
    txt  = get_forward_load_procedure(StateIndex)
    txt += '    QUEX_DEBUG_PRINT(&me->buffer, "BUFFER_RELOAD_FAILED");\n'
    txt += "    " + LanguageDB["$goto-last_acceptance"]               + "\n"
    return txt

def __reload_backward(StateIndex): 
    txt  = '    QUEX_DEBUG_PRINT(&me->buffer, "BACKWARD_BUFFER_RELOAD");\n'
    txt += "    if( QuexAnalyser_buffer_reload_backward(&me->buffer) ) {\n"
    txt += "       " + LanguageDB["$goto"]("$input", StateIndex) + "\n"
    txt += "    " + LanguageDB["$endif"]                              + "\n"
    txt += '    QUEX_DEBUG_PRINT(&me->buffer, "BUFFER_RELOAD_FAILED");\n'
    txt += "    " + LanguageDB["$goto"]("$terminal-general")          + "\n"
    return txt

def __goto_distinct_terminal(Origin):
    LanguageDB = Setup.language_db
    return LanguageDB["$goto"]("$terminal", Origin.state_machine_id)

def __goto_distinct_terminal_direct(Origin):
    LanguageDB = Setup.language_db
    return LanguageDB["$goto"]("$terminal-direct", Origin.state_machine_id)

def get_drop_out_goto_string(state, StateIdx, SM, BackwardLexingF):
    assert state.__class__.__name__ == "State"
    assert SM.__class__.__name__    == "StateMachine"
    LanguageDB = Setup.language_db

    if not BackwardLexingF:
        # (*) forward lexical analysis
        if not state.is_acceptance():
            # -- non-acceptance state drop-outs
            return LanguageDB["$goto-last_acceptance"]

        else:
            if acceptance_info.do_subsequent_states_require_storage_of_last_acceptance_position(StateIdx, state, SM):
                callback = __goto_distinct_terminal
            else:
                callback = __goto_distinct_terminal_direct
            # -- acceptance state drop outs
            return acceptance_info.get_acceptance_detector(state.origins().get_list(), callback)

    else:
        # (*) backward lexical analysis
        #     During backward lexing, there are no dedicated terminal states. Only the flags
        #     'pre-condition' fullfilled are set at the acceptance states. On drop out we
        #     transit to the general terminal state (== start of forward lexing).
        return LanguageDB["$goto"]("$terminal-general", True) 

