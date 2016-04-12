from copy import deepcopy

import quex.core_engine.generator.languages.core  as languages
from   quex.core_engine.generator.languages.core  import __nice
import quex.core_engine.generator.state_coder as state_coder
from   quex.input.setup import setup as Setup

LanguageDB = Setup.language_db

def do(SMD):
    """Returns the program code implementing the StateMachine's behavior.
       NOTE: This function should only be called on a DFA after the call
             to 'filter_dominated_origins'. The latter is important
             to ensure that for an acceptance state there is only one
             related original state.

       The procedure for each state is the following:
            i)  input <-- next character from stream 
            ii) state transition code (include marking of last success state
                and last success stream position).                  
    """
    assert SMD.__class__.__name__ == "StateMachineDecorator"

    state_machine = SMD.sm()

    txt = ""
    # -- treat initial state separately 
    if state_machine.is_init_state_a_target_state():
        # (only define the init state label, if it is really needed)
        txt += LanguageDB["$label-def"]("$entry", state_machine.init_state_index) + "\n"

    init_state = state_machine.states[state_machine.init_state_index]
    #
    # NOTE: Only the init state provides a transition via 'EndOfFile'! In any other
    #       case, end of file needs to cause a drop out! After the drop out, lexing
    #       starts at furthest right before the EndOfFile and the init state transits
    #       into the TERMINAL_END_OF_FILE.
    txt += LanguageDB["$label-def"]("$entry", state_machine.init_state_index) + "\n"
    txt += state_coder.do(init_state, 
                          state_machine.init_state_index, SMD, InitStateF = True)

    # -- all other states
    for state_index, state in state_machine.states.items():
        # the init state has been coded already
        if state_index == state_machine.init_state_index: continue

        state_code = state_coder.do(state, state_index, SMD)

        # some states are not coded (some dead end states)
        if state_code == "": continue

        txt += LanguageDB["$label-def"]("$entry", state_index) + "\n"
        txt += state_code
        txt += "\n"
    
    return txt





