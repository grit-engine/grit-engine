from quex.core_engine.state_machine.core import StateMachine
import quex.core_engine.state_machine.setup_post_context as setup_post_context
import quex.core_engine.state_machine.nfa_to_dfa as nfa_to_dfa

def do(sm, BeginOfLineF, EndOfLineF, DOS_CarriageReturnNewlineF=False):
    """DOS_CarriageReturnNewlineF == True:  
               '$' is implemented as post-condition '\r\n'. This is required
               or lexical analysers on DOS and Windows machines.
       DOS_CarriageReturnNewlineF == False:
               '$' is implemented as post-condition '\n' -- like the normal
               newline on Unix machines.
    """
    # (1) end of line 
    #     NOTE: This must come before 'Begin of File', because there's a post condition
    #           added, that enters new acceptance states.
    if EndOfLineF:
        if sm.core().post_context_id() == -1L:
            # -- create a state machine that represents the post-condition
            # -- mount it to the core pattern as a post-condition
            post_sm = StateMachine()
            if not DOS_CarriageReturnNewlineF:
                state_idx = post_sm.add_transition(post_sm.init_state_index, ord('\n'), AcceptanceF=True)
            else:
                aux_idx   = post_sm.add_transition(post_sm.init_state_index, ord('\r'), AcceptanceF=False)
                state_idx = post_sm.add_transition(aux_idx, ord('\n'), AcceptanceF=True)
            ## post_sm.add_transition(post_sm.init_state_index, EndOfFile_Code, state_idx, AcceptanceF=True)
            
            # post conditions add an epsilon transition that has to be solved 
            # by translating state machine into a DFA
            sm = setup_post_context.do(sm, post_sm) 
            sm = nfa_to_dfa.do(sm)
            assert sm.has_origins() == False
            
        else:
            post_context_id = sm.core().post_context_id()
            # end of line in two cases:
            #  (1) next char is '\n' (or \r\n in case of DOS_CarriageReturnNewlineF==True)
            #  (2) at end of file, we supposed anyway that in this case the buffer needs to
            #      end with 'EndOfFile_Code' just before the first letter.
            #
            #  => mount 'newline or EndOfFile_Code' to the tail of pattern
            #
            new_state_idx = __add_line_border_at_end(sm, 
                                                     DOS_CarriageReturnNewlineF, InverseF=False)
            # -- the post-context flag needs to be raised
            sm.states[new_state_idx].core().set_post_context_id(post_context_id)

    # (2) begin of line
    if BeginOfLineF: 
        # begin of line in two cases:
        #  (1) last char was '\n'
        #  (2) the first character is not detected as begin of line, if the 
        #      pre-condition is non-trivial.
        #
        #  A line begins always after '\n' so no check for '\r\n' is necessary.
        #  => DOS_CarriageReturnNewlineF = False
        if sm.core().pre_context_sm() != None:
            __add_line_border_at_end(sm.core().pre_context_sm(), 
                                     DOS_CarriageReturnNewlineF=False, InverseF=True)
        else:
            # mark all acceptance states with the 'trivial pre-condition BOL' flag
            for state in sm.get_acceptance_state_list():
                state.core().set_pre_context_begin_of_line_f()
            sm.core().set_pre_context_begin_of_line_f()
                
    return sm

def __add_line_border_at_end(the_sm, DOS_CarriageReturnNewlineF, InverseF):     
    """Adds the condition 'newline or border character' at the end 
       of the given state machine. Acceptance is only reached when 
       the newline or border occurs. 
       
       This function is used for begin of line or end of line pre-conditions, 
       thus: IT DOES NOT SETUP A POST-CONDITITION in the sense
       that output is scanned but cursor is being reset after match!
       The caller provides the post-condition modifications itself, if needed.

       We simply append to each acceptance state the trigger
       '\n' or BorderCharacter that leads to the new acceptance.
       The old acceptance state is annulated.
    """    
    old_acceptance_state_list = the_sm.get_acceptance_state_list() 
    new_state_idx             = the_sm.create_new_state(AcceptanceF=True)
    for state in old_acceptance_state_list:
        # (1) Transition '\n' --> Acceptance
        state.add_transition(ord('\n'), new_state_idx)
        
        if DOS_CarriageReturnNewlineF:
            # (3) Transition '\r\n' --> Acceptance
            aux_idx = the_sm.create_new_state(AcceptanceF=False)
            if not InverseF:
                state.add_transition(ord('\r'), aux_idx)
                the_sm.states[aux_idx].add_transition(ord('\n'), new_state_idx)
            else:
                state.add_transition(ord('\n'), aux_idx)
                the_sm.states[aux_idx].add_transition(ord('\r'), new_state_idx)

        # (-) Cancel acceptance of old state
        state.set_acceptance(False)
        state.core().set_store_input_position_f(False)
        state.core().set_post_context_id(-1L)
        state.core().set_pre_context_begin_of_line_f(False)
        #
    return new_state_idx    
