import quex.core_engine.generator.languages.core   as languages
from   quex.core_engine.generator.languages.core   import __nice
import quex.core_engine.generator.transition_block as transition_block
import quex.core_engine.generator.transition       as transition
import quex.core_engine.generator.acceptance_info  as acceptance_info
import quex.core_engine.generator.drop_out         as drop_out
from   quex.input.setup                            import setup as Setup
from copy import deepcopy

LanguageDB = Setup.language_db

__DEBUG_CHECK_ACTIVE_F = False # Use this flag to double check that intervals are adjacent

def do(state, StateIdx, SMD, InitStateF=False):
    """Produces code for all state transitions. Programming language is determined
       by 'Language'.
    """    
    assert SMD.__class__.__name__ == "StateMachineDecorator"

    # (*) check that no epsilon transition triggers to a real state                   
    if __DEBUG_CHECK_ACTIVE_F:
        assert state.transitions().get_epsilon_target_state_index_list() == [], \
               "epsilon transition contained target states: state machine was not made a DFA!\n" + \
               "epsilon target states = " + repr(state.transitions().get_epsilon_target_state_index_list())

    if SMD.dead_end_state_db().has_key(StateIdx):
        return transition.do_dead_end_router(state, StateIdx, SMD.backward_lexing_f())
       
    TriggerMap = state.transitions().get_trigger_map()
    assert TriggerMap != []  # Only dead end states have empty trigger maps.
    #                        # ==> here, the trigger map cannot be empty.
    #_________________________________________________________________________________________    
    # NOTE: The first entry into the init state is a little different. It does not 
    #       increment the current pointer before referencing the character to be read. 
    #       However, when the init state is entered during analysis else, the current 
    #       pointer needs to be incremented.
    txt = ""
    # note down information about success, if state is an acceptance state
    txt += input_block(StateIdx, InitStateF, SMD.backward_lexing_f())

    txt += acceptance_info.do(state, StateIdx, SMD)

    txt += transition_block.do(TriggerMap, StateIdx, InitStateF, SMD)

    txt += drop_out.do(state, StateIdx, SMD, InitStateF) 

    # Define the entry of the init state after the init state itself. This is so,
    # since the init state does not require an increment on the first beat. Later on,
    # when other states enter here, they need to increase/decrease the input pointer.
    if not SMD.backward_lexing_f():
        if InitStateF:
            txt += LanguageDB["$label-def"]("$input", StateIdx)
            txt += "    " + LanguageDB["$input/increment"] + "\n"
            txt += "    " + LanguageDB["$goto"]("$entry", StateIdx) + "\n"

    
    return txt # .replace("\n", "\n    ") + "\n"

def input_block(StateIdx, InitStateF, BackwardLexingF):
    # The initial state starts from the character to be read and is an exception.
    # Any other state starts with an increment (forward) or decrement (backward).
    # This is so, since the beginning of the state is considered to be the 
    # transition action (setting the pointer to the next position to be read).
    txt = ""
    if not BackwardLexingF:
        if not InitStateF:
            txt += LanguageDB["$label-def"]("$input", StateIdx) + "\n"
            txt += "    " + LanguageDB["$input/increment"] + "\n"
    else:
        txt += LanguageDB["$label-def"]("$input", StateIdx) + "\n"
        txt += "    " + LanguageDB["$input/decrement"] + "\n"

    txt += "    " + LanguageDB["$input/get"] + "\n"

    return txt

