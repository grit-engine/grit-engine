
def do(state_machine):
    """(1) Delete transitions to non-acceptance states that have no further 
           transition. Delete also the correspondent empty states.

           This can be done, since a transition to a non-acceptance state
           where there is no further path is identical to a drop-out. The
           drop out happens if no transition happens on current character.
       
       (2) Collect all states that have no further transitions, i.e. dead end states.
           Some of them need to be investigated, since it depends on pre-conditions
           what terminal state is to be considered. The mapping works as follows:

            db.has_key(state_index) == False    ==>  state is not a dead end at all.

            db[state_index] = None              ==> state is a 'gateway' please, jump as usual to 
                                                     the state, the state then routes to the correspondent
                                                     terminal (without input) depending on the 
                                                     pre-conditions.

            db[state_index] = integer           ==> state transits immediately to TERMINAL_n
                                                     where 'n' is the integer.

            db[state_index] = -1                ==> state is a 'harmless' drop out. need to jump to
                                                    general terminal
    """
    db = {}
    directly_reached_terminal_id_list = []
    non_acceptance_dead_end_state_index_list = []
    for state_index, state in state_machine.states.items():

        if not state.transitions().is_empty(): continue

        if state.is_acceptance(): 
            # (2) Acceptance States --> short cuts
            db[state_index] = state
            if state.origins().contains_any_pre_context_dependency():
                # (a) state require run time investigation since pre-conditions have to be checked
                #     Terminals are reached via a 'router'. nevertheless, they are reached 
                #     without a seek.
                for origin in state.origins().get_list():
                    if not origin.is_acceptance(): continue
                    directly_reached_terminal_id_list.append(origin.state_machine_id)

            else:
                # Find the first acceptance origin (origins are sorted already)
                acceptance_origin = state.origins().find_first_acceptance_origin()
                # There **must** be an acceptance state, see above
                assert type(acceptance_origin) != type(None)

                # (b) state transits automatically to terminal given below
                directly_reached_terminal_id_list.append(acceptance_origin.state_machine_id)
        else:
            # (1) Non-Acceptance states --> deletion of state and all transitions to it
            for state in state_machine.states.values():
                state.transitions().delete_transitions_to_target(state_index) 

            del state_machine.states[state_index]
            non_acceptance_dead_end_state_index_list.append(state_index)

    return db, directly_reached_terminal_id_list
