#! /usr/bin/env python
import sys
from copy import deepcopy
sys.path.append("../")
sys.path.append("../../")

import quex.core_engine.state_machine
from quex.core_engine.state_machine.core import StateMachine

def do(the_state_machines):
    """Connect state machines paralell."""
    assert type(the_state_machines) == list
    assert len(the_state_machines) != 0
    assert map(lambda x: x.__class__.__name__, the_state_machines) == ["StateMachine"] * len(the_state_machines)
              
    # filter out empty state machines from the consideration          
    state_machines = filter(lambda sm: not sm.is_empty(), the_state_machines)

    def __add_optional_free_pass(result_state_machine,
                                 TerminationStateIdx=-1):
        """Add an optional 'free pass' if there was an empty state."""  
        # if there was an empty state, then the number of elements in the list changed
        # in case there was an empty state one has to add a 'free pass' from begin to 
        # the final acceptance state.   
        if TerminationStateIdx == -1:
            acceptance_state_index_list = result_state_machine.get_acceptance_state_index_list()
            assert acceptance_state_index_list != [], \
                   "resulting state machine has no acceptance state!"
            TerminationStateIdx = acceptance_state_index_list[0]

        if len(state_machines) != len(the_state_machines):
            result_state_machine.add_epsilon_transition(result_state_machine.init_state_index, 
                                                        TerminationStateIdx)
        return result_state_machine

    if len(state_machines) < 2:
        if len(state_machines) < 1: return __add_optional_free_pass(StateMachine())
        else:                       return __add_optional_free_pass(state_machines[0])

    # (*) need to clone the state machines, i.e. provide their internal
    #     states with new ids, but the 'behavior' remains. This allows
    #     state machines to appear twice, or being used in 'larger'
    #     conglomerates.
    clone_list = map(lambda sm: sm.clone(), state_machines)

    # (*) collect all transitions from both state machines into a single one
    #     (clone to ensure unique identifiers of states)
    result = StateMachine()
    for clone in clone_list:
        for start_state_index, states in clone.states.items():        
            # DOUBT: is deepcopy necessary at this place?
            # ANSWER: it does not harm, because no new state indices are creates
            result.states[start_state_index] = deepcopy(states)

    # (*) add additional **init** and **end** state
    #     NOTE: when the result state machine was created, it already contains a 
    #           new initial state index. thus at this point only the new terminal
    #           state has to be created. 
    #     NOTE: it is essential that the acceptance flag stays False, at this
    #           point in time, so that the mounting operations only happen on
    #           the old acceptance states. Later the acceptance state is raised
    #           to 'accepted' (see below)
    new_terminal_state_index = result.create_new_state() 
    
    # (*) connect from the new initial state to the initial states of the
    #     clones via epsilon transition. 
    #     connect from each success state of the clones to the new end state
    #     via epsilon transition.
    for clone in clone_list:
        result.mount_to_initial_state(clone.init_state_index)
        result.mount_to_acceptance_states(new_terminal_state_index,
                                          CancelStartAcceptanceStateF=True,
                                          RaiseTargetAcceptanceStateF=True,
                                          LeaveStoreInputPositionsF=True)


    return __add_optional_free_pass(result, new_terminal_state_index)

