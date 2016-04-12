#! /usr/bin/env python
import sys
from copy import deepcopy
sys.path.append("../")

from core import *


def do(the_state_machine_list, LeaveIntermediateAcceptanceStatesF=False, 
       MountToFirstStateMachineF=False, CloneRemainingStateMachinesF=True):
    """Creates a state machine connecting all state machines in the array 
       'state_machine_list'. When the flag 'LeaveIntermediateAcceptanceStatesF'
       is given as True, the connection points between the state machines
       will remain  acceptances states. In any other case (e.g. the normal
       sequentialization) the connection points leave there acceptance 
       status and only the last state machine in the list keeps its
       acceptance states.

       If MountToFirstStateMachineF is set, then the first state machine will
       contain the result of the concatination.
    """
    assert type(the_state_machine_list) == list 
    assert len(the_state_machine_list) != 0
    assert map(lambda x: x.__class__.__name__, the_state_machine_list) == ["StateMachine"] * len(the_state_machine_list)

    for sm in the_state_machine_list:   # DEBUG
        sm.assert_consistency()         # DEBUG

    # state machines with no states can be deleted from the list. they do not do anything
    # and do not introduce triggers.          
    state_machine_list = filter(lambda sm: not sm.is_empty(), the_state_machine_list)         
   
    if len(state_machine_list) < 2:
        if len(state_machine_list) < 1: return StateMachine()
        else:                           return state_machine_list[0]

    # (*) collect all transitions from both state machines into a single one
    #     (clone to ensure unique identifiers of states)
    result = state_machine_list[0]
    if not MountToFirstStateMachineF:  result = result.clone()

    # (*) need to clone the state machines, i.e. provide their internal
    #     states with new ids, but the 'behavior' remains. This allows
    #     state machines to appear twice, or being used in 'larger'
    #     conglomerates.
    appended_sm_list = state_machine_list[1:]
    if CloneRemainingStateMachinesF: 
        appended_sm_list = map(lambda sm: sm.clone(), appended_sm_list)

    # (*) all but last state machine enter the subsequent one, in case of SUCCESS
    #     NOTE: The start index is unique. Therefore, one can assume that each
    #           appended_sm_list's '.states' dictionary has different keys. One can simply
    #           take over all transitions of a start index into the result without
    #           considering interferences (see below)
    for appendix in appended_sm_list:
        appendix.assert_consistency() # DEBUG
        # Mount on every acceptance state the initial state of the following state
        # machine via epsilon transition.
        result.mount_to_acceptance_states(appendix.init_state_index, 
                                          CancelStartAcceptanceStateF = not LeaveIntermediateAcceptanceStatesF)
        for state_index, state in appendix.states.items():        
            result.states[state_index] = state # state is already cloned (if desired), so no deepcopy here

    # (*) double check for consistency (each target state is contained in state machine)
    result.assert_consistency() # DEBUG
    return result
