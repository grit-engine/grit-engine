#! /usr/bin/env python
import sys
from   quex.frs_py.file_in import error_msg
from   quex.core_engine.state_machine.core import *
import quex.core_engine.state_machine.ambiguous_post_context as apc


def do(the_state_machine, post_context_sm, DEMONSTRATION_TurnOffSpecialSolutionF=False, fh=-1):
    """ Appends a post context to the given state machine. This process is very
        similar to sequentialization. There is a major difference, though:
       
        Given a state machine (e.g. a pattern) X with a post context Y, 
        a match is only valid if X is followed by Y. Let Xn be an acceptance
        state of X and Ym an acceptance state of Y: 

               ---(Xn-1)---->(Xn)---->(Y0)----> ... ---->((Ym))
                             store                       acceptance
                             input
                             position
        
        That is, it holds:

           -- The next input position is stored the position of Xn, even though
              it is 'officially' not an acceptance state.

           -- Ym will be an acceptance state, but it will not store 
              the input position!       

        The analysis of the next pattern will start at the position where
        X stopped, even though Ym is required to state acceptance.    
       
    """
    # (*) do some consistency checking   
    assert the_state_machine.__class__.__name__ == "StateMachine", \
            "expected 1st argument as objects of class StateMachine\n" + \
            "received: " + the_state_machine.__class__.__name__
    assert post_context_sm.__class__.__name__ == "StateMachine", \
            "expected 2nd argument as objects of class StateMachine\n" + \
            "received: " + post_context_sm.__class__.__name__
    assert the_state_machine.core().post_context_id() == -1L, \
            "post context state machine cannot be post-context again."

    # -- state machines with no states are senseless here. 
    assert not the_state_machine.is_empty(), \
            "empty state machine can have no post context."
    assert not post_context_sm.is_empty(), \
            "empty state machine cannot be a post-context."

    # -- state machines involved with post condition building are part of a pattern, 
    #    but not configured out of multiple patterns. Thus there should be no origins.
    assert the_state_machine.has_origins() == False
    assert post_context_sm.has_origins() == False

    # -- a post context with an initial state that is acceptance is not
    #    really a 'context' since it accepts anything. The state machine remains
    #    un-post context.
    if post_context_sm.get_init_state().is_acceptance():
        error_msg("Post context accepts anything---replaced by no post context.", fh, 
                  DontExitF=True)
        return the_state_machine
    
    # (*) Two ways of handling post-contexts:
    #
    #     -- Seldom Exception: 
    #        Pseudo-Ambiguous Post Conditions (x+/x) -- detecting the end of the 
    #        core pattern after the end of the post context
    #        has been reached.
    #
    if not DEMONSTRATION_TurnOffSpecialSolutionF:
        if apc.detect_forward(the_state_machine, post_context_sm):
            if apc.detect_backward(the_state_machine, post_context_sm):
                # -- for post contexts that are forward and backward ambiguous
                #    a philosophical cut is necessary.
                error_msg("Post context requires philosophical cut---this operation is not reliable.", fh, 
                          DontExitF=True)
                post_context_sm = apc.philosophical_cut(the_state_machine, post_context_sm)

            apc.mount(the_state_machine, post_context_sm)
            return the_state_machine

    #     -- The 'normal' way: storing the input position at the end of the core
    #        pattern.
    #
    result     = the_state_machine
    # (*) need to clone the state machines, i.e. provide their internal
    #     states with new ids, but the 'behavior' remains. This allows
    #     state machines to appear twice, or being used in 'larger'
    #     conglomerates.
    post_clone = post_context_sm.clone() 

    # (*) collect all transitions from both state machines into a single one
    #
    #     NOTE: The start index is unique. Therefore, one can assume that each
    #           clone_list '.states' dictionary has different keys. One can simply
    #           take over all transitions of a start index into the result without
    #           considering interferences (see below)
    #
    orig_acceptance_state_id_list = result.get_acceptance_state_index_list()

    # -- mount on every acceptance state the initial state of the following state
    #    machine via epsilon transition
    result.mount_to_acceptance_states(post_clone.init_state_index, CancelStartAcceptanceStateF=True)
    for start_state_index, state in post_clone.states.items():        
        result.states[start_state_index] = state # states are already cloned

    # -- consider the pre-context, it has to be moved to the acceptance state
    # -- same for trivial pre-contexts        
    post_context_id             = the_state_machine.core().id()
    pre_context_sm              = the_state_machine.core().pre_context_sm()
    pre_context_sm_id           = the_state_machine.core().pre_context_sm_id()
    pre_context_begin_of_line_f = the_state_machine.core().pre_context_begin_of_line_f() 

    # -- raise at each old acceptance state the 'store input position flag'
    # -- set the post context flag for all acceptance states
    for state_idx in orig_acceptance_state_id_list:
        state = result.states[state_idx]
        state.core().set_store_input_position_f(True)
        state.core().set_post_context_id(post_context_id)
        state.core().set_pre_context_id(-1L)   
    
    # -- no acceptance state shall store the input position
    # -- set the post context flag for all acceptance states
    for state in result.get_acceptance_state_list():
        state.core().set_store_input_position_f(False)
        state.core().set_post_context_id(post_context_id)
        state.core().set_pre_context_id(pre_context_sm_id)   

    # -- information about the pre-context remains
    result.core().set_pre_context_begin_of_line_f(pre_context_begin_of_line_f)
    result.core().set_pre_context_sm(pre_context_sm)
    result.core().set_post_context_id(post_context_id)

    return result
