#! /usr/bin/env python
# PURPOSE: creates a state machine that is the repeated version of the 
#          given one. this means, that the success state is linked to the 
#          start state.
import sys
from copy import deepcopy
sys.path.append("../")

from core import *
import sequentialize

def do(the_state_machine, min_repetition_n = 0, max_repetition_n = -1):
    """ Creates a state machine that represents a repetition of the given 
        'the_state_machine'. Minimum and maximim number of repetitions can be specified.
    """
    assert min_repetition_n <= max_repetition_n or max_repetition_n == -1

    # (*) if minimum number of repetitions is required, then the initial
    #     repetition is produced by sequentialization.
    initial_state_machine = None
    if min_repetition_n != 0:
        # Concatinate the state machine N times, at the beginning, so that 
        # there are N repetitions at least. Any 'bail out' before the first N
        # repetitions happen from a 'non-acceptance' state => fail. Only, when
        # the first N repetitions happend, the state machines enters into the
        # following 'repetition states'.
        # NOTE: sequentialize clones the given state machines 
        initial_state_machine = sequentialize.do([the_state_machine] * min_repetition_n)

    if max_repetition_n != -1:
        # if a maximum number of repetitions is given, then the state machine needs 
        # to be repeated 'physically'. No new 'repeated' version of the state machine
        # is computed.
        # NOTE: sequentialize clones the given state machines 
        if initial_state_machine != None: 
            return sequentialize.do([initial_state_machine] 
                                    + [the_state_machine] * (max_repetition_n - min_repetition_n),
                                    LeaveIntermediateAcceptanceStatesF = True)
        else:
            concatenation = sequentialize.do([the_state_machine] * max_repetition_n,
                                             LeaveIntermediateAcceptanceStatesF = True)
            # Here, zero initial repetitions are required, thus the initial state must be
            # an acceptance state.
            concatenation.states[concatenation.init_state_index].set_acceptance(True)                                      
            return concatenation

    # (*) clone the state machine
    #     NOTE: kleene_closure() clones the state machine.
    pure_repetition = kleene_closure(the_state_machine)
    if initial_state_machine != None: 
        return sequentialize.do([initial_state_machine] + [ pure_repetition ],
                                LeaveIntermediateAcceptanceStatesF = True)
    else:
        return pure_repetition

def kleene_closure(the_state_machine):
    """Creates a state machine that is repeated any number of times 
       (zero is also accepted).
       See: Thomson construction.
    """    
    # (*) clone the machine
    result = the_state_machine.clone()

    # (*) add additional initial state
    prev_init_state_index  = result.init_state_index
    result.create_new_init_state() 
    result.mount_to_initial_state(prev_init_state_index)
    # print "##rmounti:", result

    # (*) add additional terminal state
    new_terminal_state_index = result.create_new_state()

    # (*) connect all acceptance states via epsilon transition 
    #     *backwards* to old initial state.
    #
    #     NOTE: do not cancel the acceptance state of any acceptance state,
    #           so the next step can enter another target state index.
    result.mount_to_acceptance_states(prev_init_state_index,
                                      CancelStartAcceptanceStateF=False,
                                      RaiseTargetAcceptanceStateF=False)
    # print "##backward:", result
    # (*) connect all acceptance states via epsilon transition 
    #     *forwards* to terminal state
    result.mount_to_acceptance_states(new_terminal_state_index,
                                      CancelStartAcceptanceStateF=True,
                                      RaiseTargetAcceptanceStateF=False)
    # print "##forward:", result

    # (*) add epsilon transition from new init state to new terminal state
    result.add_epsilon_transition(result.init_state_index, new_terminal_state_index, 
                                  RaiseAcceptanceF=True)    

    return result


