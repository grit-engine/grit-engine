# PURPOSE: Quex's strategy to handle post-conditions is the following:
#          When a state is reached that is labeled with 'end of core'
#          it stores the input position in a certain variable. This
#          approach is 'pure' in a sense that it does not circumvent the
#          understanding of a regular expression based state machine.
# 
#          A problem arises with some post-conditions, usually called
#          'dangerous trailing context'. Some of those cases can be 
#          handled by quex. Other's are by nature ambiguous. This
#          file provides functions to analyse the different cases.
# 
# (C) Frank-Rene Schaefer
##############################################################################

from   quex.core_engine.interval_handling import NumberSet, Interval
import quex.core_engine.state_machine.sequentialize         as sequentialize
import quex.core_engine.state_machine.nfa_to_dfa            as nfa_to_dfa
import quex.core_engine.state_machine.hopcroft_minimization as hopcroft
import sys
from   copy import deepcopy


def detect_forward(CoreStateMachine, PostConditionStateMachine):
    """A 'forward ambiguity' denotes a case where the quex's normal
       post condition implementation fails. This happens if an
       iteration in the core pattern is a valid path in the post-
       condition pattern. In this case no decision can be be where
       to reset the input position.

       Example:   x+/x  At the end of the post condition an incoming
                        'x' guides through a path in the post condition
                        and the core pattern. It cannot be determined
                        by a flag where the input position ends.

       NOTE: For many cases where there is a forward ambiguity quex
       can gnerate an inverse post-condition that goes backwards from 
       the end of the post condition (see function 'mount()'). However,
       there are cases where even this is not possible (see function
       'detect_backward()').
    """
    __assert_state_machines(CoreStateMachine, PostConditionStateMachine)
    
    core_acceptance_state_list = CoreStateMachine.get_acceptance_state_list()

    pcsm_init_state = PostConditionStateMachine.get_init_state()
    for csm_state in core_acceptance_state_list:
        if  __dive_to_detect_iteration(CoreStateMachine,          csm_state, 
                                       PostConditionStateMachine, pcsm_init_state):
            return True

    return False

def detect_backward(CoreStateMachine, PostConditionStateMachine):

    """A 'backward ambiguity' denotes the case where it cannot be clearly be
       determined how far to go back from the end of a post-condition. 
       
       NOTE: This does not mean that the post-condition is ambiguous. Many
       cases that are backward ambiguous can be handled by quex's normal
       post-condition handling.

       Examples:  x/x+   is backward ambiguous because in a stream
                         of 'x' one cannot determine with a pure
                         state machine where to stop. This case,
                         though can be handled by the normal post-
                         condition implementation.

                  x+/x+  is backward ambiguous and cannot be handled
                         by the normal implementation. In fact, this
                         specification does not allow any conclusions
                         about the users intend where to reset the 
                         input after match.
    """

    __assert_state_machines(CoreStateMachine, PostConditionStateMachine)

    my_post_context_sm = PostConditionStateMachine.clone()

    # (*) Create a modified version of the post condition, where the
    #     initial state is an acceptance state, and no other. This 
    #     allows the detector to trigger on 'iteration'.
    #
    # -- delete all acceptance states in the post condition
    # for state in my_post_context_sm.states.values():
    #   state.set_acceptance(False)
    # -- set the initial state as acceptance state
    # my_post_context_sm.get_init_state().set_acceptance(True)

    my_core_sm = CoreStateMachine.get_inverse()
    my_core_sm = nfa_to_dfa.do(my_core_sm)
    my_core_sm = hopcroft.do(my_core_sm)

    tmp = deepcopy(PostConditionStateMachine)
    my_post_context_sm = tmp.get_inverse()
    my_post_context_sm = nfa_to_dfa.do(my_post_context_sm)
    my_post_context_sm = hopcroft.do(my_post_context_sm)

    return detect_forward(my_post_context_sm, my_core_sm)

def __dive_to_detect_iteration(SM0, sm0_state, SM1, sm1_state):
    """This function goes along all path of SM0 that lead to an 
       acceptance state AND at the same time are valid inside SM1.
       The search starts at the states sm0_state and sm1_state.
    """

    sm0_transition_list = sm0_state.transitions().get_map().items()
    sm1_transition_list = sm1_state.transitions().get_map().items()

    # If there is no subsequent path in SM0 or SM1, 
    # then we are at a leaf of the tree search. No
    # path to acceptance in SM0 lies in SM1.
    if sm0_transition_list == [] or sm1_transition_list == []:
        return False

    for sm0_target_state_index, sm0_trigger_set in sm0_transition_list:
        for sm1_target_state_index, sm1_trigger_set in sm1_transition_list:
            # If there is no common character in the transition pair, it does not
            # have to be considered.
            if not sm0_trigger_set.has_intersection(sm1_trigger_set):
                continue
            
            # Both trigger on the some same characters.
            #     -----------------------[xxxxxxxxxxxxxxxx]-------------
            #     -------------[yyyyyyyyyyyyyyyyyyyy]-------------------
            #
            # If the target state in the SM0 is an acceptance state,
            # => A valid path in SM1 leads at the same time to along 
            #    valid path in SM0.
            sm0_target_state = SM0.states[sm0_target_state_index]
            if sm0_target_state.is_acceptance():
                return True
            
            # If the state is not immediately an acceptance state, then
            # search in the subsequent pathes of the SM0.
            sm1_target_state = SM1.states[sm1_target_state_index]
            if __dive_to_detect_iteration(SM0, sm0_target_state, SM1, sm1_target_state):
                return True

    # None of the investigated paths in SM0 and SM1 leads to an
    # acceptance state in SM0. 
    return False
    
def __get_inverse_state_machine_that_finds_end_of_core_expression(PostConditionSM):
    """In case of a pseudo-ambiguous post condition one needs to go backwards
       in order to search for the end of the core condition. This function 
       creates the inverse state machine that is able to go backwards.

       NOTE: This is a special case, because one already knows that the state
       machine reaches the acceptance state sometime (this is where it actually
       started). That means, that in states other than acceptance states one
       can take out the 'drop out' triggers since they CANNOT occur. This
       enables some speed-up when going backwards.
    """
    result = PostConditionSM.get_inverse()
    result = nfa_to_dfa.do(result)
    result = hopcroft.do(result)

    # -- delete 'drop-out' transitions in non-acceptance states
    #    NOTE: When going backwards one already knows that the acceptance
    #          state (the init state of the post condition) is reached, see above.
    for state in result.states.values():
        # -- acceptance states can have 'drop-out' (actually, they need to have)
        if state.is_acceptance(): continue

        state.transitions().replace_drop_out_target_states_with_adjacent_targets()

    result = nfa_to_dfa.do(result)
    result = hopcroft.do(result)

    # Acceptance States need to be marked: Store input position.
    # NOTE: When tracing backwards the match is guaranteed, but there might
    #       still be some 'trail' in case of iterations that are not directly
    #       iterated to the ambiguous post condition. Thus drop out may
    #       happen and it must be clear where to put the input pointer in this case.

    return result

def mount(the_state_machine, PostConditionSM):
    """This function mounts a post condition to a state machine with
       a mechanism that is able to handle the pseudo ambigous post-
       condition. Note, that this mechanism can also treat 'normal'
       post-conditions. However, it is slightly less efficient.

                core-        post-    
           -----0000000000000111111111--------------

       (1)      |-------------------->
                                     acceptance

       (2)                   <-------|
                             reset input position

       The first step is performed by 'normal' lexing. The second step
       via the backward detector, which is basically an inverse state
       machine of the post-condition.

       NOTE: This function does **not** return a state machine that is
             necessarily deterministic. Run nfa_to_dfa on the result
             of this function.

       NOTE: This function is very similar to the function that mounts
             a pre-condition to a state machine. The only major difference
             is that the post condition is actually webbed into the 
             state machine for forward lexing. For backward lexing
             a reference is stored that points to the backward detecting
             state machine.
    """
    assert the_state_machine.__class__.__name__ == "StateMachine"
    assert PostConditionSM.__class__.__name__ == "StateMachine"
    # -- state machines with no states are senseless here. 
    assert not the_state_machine.is_empty() 
    assert not PostConditionSM.is_empty()

    # -- trivial pre-conditions should be added last, for simplicity
    # (*) concatinate the two state machines:
    #   -- deletes acceptance states of the core pattern
    #   -- leaves acceptance states of the post condition
    sequentialize.do([the_state_machine, PostConditionSM], MountToFirstStateMachineF=True)

    # (*) get the state machine that can go backwards from the acceptance
    #     state of the post condition to the start of the post-condition.
    #     The start of the post condition is at the same time the end 
    #     of the core pattern.
    backward_detector_sm    = __get_inverse_state_machine_that_finds_end_of_core_expression(PostConditionSM)
    backward_detector_sm_id = backward_detector_sm.get_id()

    # NOTE: We do not need to mark any origins in the backward detector,
    #       since it is not concerned with acceptance states. Its only
    #       task is to reset the input stream.
    # NOTE: It is not necessary that the state machine directly refers to
    #       the backward detector. The origins of the acceptance state will do so.
    acceptance_state_list = the_state_machine.get_acceptance_state_list()
    assert len(acceptance_state_list) != 0, \
            "error: mounting pseudo-ambiguous post condition:\n" + \
            "error: no acceptance state in sequentialized state machine."

    # (*) Create origin data, in case where there is none yet create new one.
    #     (Do not delete, otherwise existing information gets lost.)
    for state in acceptance_state_list: 
        state.core().set_post_context_backward_detector_sm_id(backward_detector_sm_id)
        # At the end of the post condition, the input positions needs to be stored. Before
        # we can go backwards, we need to know where the post condition actually ended.
        state.core().set_store_input_position_f(True)

    the_state_machine.core().set_post_context_backward_input_position_detector_sm(backward_detector_sm)


    # We cannot do a NFA to DFA and Hopcroft Optimization, because otherwise we
    # would create a new state machine. This function, though, is considered to 
    # 'mount' something on an existing state machine, i.e. change the object
    # that is referenced by the first argument.
    return the_state_machine

def philosophical_cut(core_sm, post_context_sm):
    """The 'philosophical cut' is a technique introduced by Frank-Rene Schaefer
       to produce a pair of a core- and a post-condition that otherwise would 
       be forward and backward ambiguous. The philosophical ground for this
       cut is 'greed', i.e. a core pattern should eat as much characters as
       it can. This idea is followed during the whole construction of the lexical
       analyzer. 
       
       For the case of total ambiguity 'x+/x+', this idea translates into leaving
       the iteration in the core condition and cutting the iteration in the post
       condition. Thus 'x+/x+' is transformed into 'x+/x' and can be solved by
       the technique for forward ambiguous post conditions.
    """
    core_acceptance_state_list = core_sm.get_acceptance_state_list()

    pcsm_init_state = post_context_sm.get_init_state()
    for csm_state in core_acceptance_state_list:
        __dive_to_cut_iteration(core_sm, csm_state, post_context_sm, pcsm_init_state,
                                SM1_Path=[post_context_sm.init_state_index])

    # By means of cutting, some states might have become bold. That is, they have
    # only an epsilon transition. Thus, it is required to do a transformation NFA->DFA
    # and add a hopcroft optimization.
    new_post_sm = nfa_to_dfa.do(post_context_sm)
    new_post_sm = hopcroft.do(new_post_sm)
    return new_post_sm

def __dive_to_cut_iteration(SM0, sm0_state, SM1, sm1_state, SM1_Path):
    """Cut any trigger that allows to trigger out of SM1 that triggers 
       back to its initial state while the path is valid in SM0. This
       function serves the 'philosophical cut'.
    """
    sm0_transition_list = sm0_state.transitions().get_map().items()
    sm1_transition_list = sm1_state.transitions().get_map().items()

    # If there is no subsequent path in SM0 or SM1, then we are at a leaf of
    # the tree search. No path to acceptance in SM0 lies in SM1.
    if sm0_transition_list == [] or sm1_transition_list == []: return 

    for sm0_target_state_index, sm0_trigger_set in sm0_transition_list:
        for sm1_target_state_index, sm1_trigger_set in sm1_transition_list:

            ## print "##", intersection.get_utf8_string(), sm1_transition.target_state_index, SM1_Path
            # Both trigger on the some same characters?
            #     -----------------------[xxxxxxxxxxxxxxxx]-------------
            #     -------------[yyyyyyyyyyyyyyyyyyyy]-------------------
            if not sm0_trigger_set.has_intersection(sm1_trigger_set):
                # If there is no common character in the transition pair, it does not
                # have to be considered.
                continue
            
            elif sm1_target_state_index in SM1_Path:
                # If the trigger set allows the current state to trigger to a state
                # that has already been reached in the path of states, then this
                # is the door to iteration. This 'door' backwards needs to be locked!
                # PREVIOUSLY: intersection = sm0_trigger_set.intersection(sm1_trigger_set)
                sm1_trigger_set.subtract(sm0_trigger_set)  # PREVIOUSLY: subtract(intersection)
                sm1_state.transitions().delete_transitions_on_empty_trigger_sets()
                # (Where there is no door, there is no way to dive deeper ...)

            else:
                # Lets see to where this path may guide us ...
                sm0_target_state = SM0.states[sm0_target_state_index]
                sm1_target_state = SM1.states[sm1_target_state_index]
                __dive_to_cut_iteration(SM0, sm0_target_state, 
                                        SM1, sm1_target_state,
                                        SM1_Path + [sm1_target_state_index])

    # All branches considered, ... dive up
    return 

def __assert_state_machines(SM0, SM1):
    assert SM0.__class__.__name__ == "StateMachine"
    assert SM1.__class__.__name__ == "StateMachine"

