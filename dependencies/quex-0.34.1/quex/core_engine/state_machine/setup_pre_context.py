#! /usr/bin/env python
import sys
import os
sys.path.insert(0, os.environ["QUEX_PATH"])
from copy import deepcopy

from   quex.core_engine.state_machine.core import *
import quex.core_engine.state_machine.nfa_to_dfa as nfa_to_dfa
import quex.core_engine.state_machine.hopcroft_minimization as hopcroft

def do(the_state_machine, pre_context_state_machine):
    """Sets up a pre-condition to the given state machine. This process
       is entirely different from any sequentialization or paralellization
       of state machines. Here, the state machine representing the pre-
       condition ist **not** webbed into the original state machine!

       Instead, the following happens:

          -- the pre-condition state machine is inverted, because
             it is to be walked through backwards.
          -- the inverted state machine is marked with the state machine id
             of the_state_machine.        
          -- the original state machine will refere to the inverse
             state machine of the pre-condition.
          -- the initial state origins and the origins of the acceptance
             states are marked as 'pre-conditioned' indicating the id
             of the inverted state machine of the pre-condition.             
    """
    #___________________________________________________________________________________________
    # (*) do some consistency checking   
    assert the_state_machine.__class__.__name__ == "StateMachine"
    assert pre_context_state_machine.__class__.__name__ == "StateMachine"
    # -- state machines with no states are senseless here. 
    assert not the_state_machine.is_empty() 
    assert not pre_context_state_machine.is_empty()
    # -- trivial pre-conditions should be added last, for simplicity
    assert not the_state_machine.core().pre_context_begin_of_line_f(), \
           "This function was not designed to deal with trivially pre-conditioned state machines." + \
           "Please, make sure the trivial pre-conditioning happens *after* regular pre-conditions."  
    #___________________________________________________________________________________________
        
    # (*) invert the state machine of the pre-condition 
    inverse_pre_context = pre_context_state_machine.get_inverse()
    inverse_pre_context = nfa_to_dfa.do(inverse_pre_context)
    inverse_pre_context = hopcroft.do(inverse_pre_context)
        
    # (*) let the state machine refer to it 
    #     [Is this necessary? Is it not enough that the acceptance origins point to it? <fschaef>]
    the_state_machine.core().set_pre_context_sm(inverse_pre_context)
    pre_context_sm_id = inverse_pre_context.get_id()

    # (*) create origin data, in case where there is none yet create new one.
    #     (do not delete, otherwise existing information gets lost)
    for state in the_state_machine.states.values():
        if not state.is_acceptance(): continue
        state.core().set_pre_context_id(pre_context_sm_id)
    
    return the_state_machine
