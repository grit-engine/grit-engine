import quex.core_engine.state_machine.index as     state_machine_index
from   quex.core_engine.state_machine.core  import StateMachine
from   quex.core_engine.state_machine.index import map_state_combination_to_index

class StateSet_List:
    def __init__(self, StateMachine):
        self.sm = StateMachine
        #
        # -- map: [state index]  -->  [index of the state set that contains it]
        self.map = {} 
        #
        # -- create: self.state_set_list by initial split of all states.
        self.__initial_split()
        self.size = len(self.state_set_list)

    def get(self, Index):
        return self.state_set_list[Index]

    def split(self, StateSetIndex):
        """RETURNS:  False   if StateSet does not need to be split up any further.
                     True    if the state set requires a split.
        """
        state_set = self.state_set_list[StateSetIndex]
        #
        N         = len(state_set)
        assert N != 0, "State set of size '0'. List = " + repr(state_set_list)
        # only one state in state set => no change possible
        if N == 1: return False    

        # -- choose one arbitrary state (for example state 0) as a prototype
        #    which is compared against the remaining states in the state set.
        prototype_index      = state_set[0]
        prototype            = self.sm.states[prototype_index]
        equivalent_state_set = [ prototype_index ] 

        # -- loop over all remaining states from state set
        i         = 1   # state_set[i] = state index
        element_n = N   # remaining number of elements in state set
        for state_index in state_set[1:]:
            state = self.sm.states[state_index]
            if self.__equivalence(prototype, state): 
                equivalent_state_set.append(state_index)

        # -- Are all states equivalent?
        if len(equivalent_state_set) == N: return False  # no split! 

        # -- States that are not equivalent (probably most likely) remain in the 
        #    original state set and the ones that are equivalent are put into a new
        #    set at the end of the state set list.
        #    
        #    Delete equivalent states from the original state set
        for state_index in equivalent_state_set:
            i = state_set.index(state_index)
            del state_set[i]

        self.__add_state_set(equivalent_state_set)
        return True

    def __initial_split(self):
        """Returns the set of states that are 'acceptance'. If the optional     
           argument 'ReturnNonAcceptanceTooF' is specified, then the non-
           acceptance states are also returned.

        """   
        self.state_set_list = []

        # (1) Split according to acceptance and non-acceptance
        self.state_set_list.append([])  # state set '0': non-acceptance states
        acceptance_state_set = []       # acceptance states
        for state_index, state in self.sm.states.items():
            if state.is_acceptance(): 
                acceptance_state_set.append(state_index)
            else:                     
                self.state_set_list[0].append(state_index)  # put state into state set 0
                self.map[state_index] = 0                   # note, that it is stored in state set '0'

        # NOTE: Under normal conditions, there **must** be at least one non-acceptance state,
        #       which happens to be the initial state (otherwise nothing would be acceptable).
        #       But, for unit tests etc. we need to live with the possibility that the there 
        #       might be no non-acceptance states.
        if len(self.state_set_list[0]) == 0: del self.state_set_list[0]
        # ... size matters from now on!
        self.size = len(self.state_set_list)

        # (2) Split the acceptance states according to their origin. An acceptance
        #     state maching the, for example, an identifier is not equivalent an 
        #     acceptance state thate that matches a number.
        db = {}   
        def db_add(key, state_index):
            if db.has_key(key): db[key].append(state_index)
            else:               db[key] = [ state_index ]                             

        for state_index in acceptance_state_set:
            state = self.sm.states[state_index]
            origin_state_machine_ids = map(lambda origin: origin.state_machine_id, 
                                           state.origins().get_list())
            state_combination_id = map_state_combination_to_index(origin_state_machine_ids) 
            db_add(state_combination_id, state_index)

        # (2b) Enter the splitted acceptance state sets.
        for state_set in db.values():
            self.__add_state_set(state_set)

    def __add_state_set(self, NewStateSet):
        #    Create the new state set at the end of the list
        self.state_set_list.append(NewStateSet)
        # -- Mark in the map the states that have moved to the new state set at the end.
        for state_index in NewStateSet:
            self.map[state_index] = self.size
        # -- increase the size counter
        self.size += 1 

        return True

    def __equivalence(self, This, That):
        """Do state 'This' and state 'That' trigger on the same triggers to the
           same target state?
        """
        transition_list_0 = This.transitions().get_map().items()
        transition_list_1 = That.transitions().get_map().items()

        if len(transition_list_0) != len(transition_list_1): return False

        # Assumption: Transitions do not appear twice. Thus, both lists have the same
        #             length and any element of A appears in B, the two must be equal.
        for t0_target_state_index, t0_trigger_set in transition_list_0:
            # find transition in 'That' state that contains the same trigger set
            for t1_target_state_index, t1_trigger_set in transition_list_1:
                if t0_trigger_set.is_equal(t1_trigger_set): break
            else:
                # no trigger set found in 'That' that corresponds to 'This' => not equivalent
                return False

            target_0 = self.map[t0_target_state_index]
            target_1 = self.map[t1_target_state_index]

            # do both states trigger on the same trigger set to the same target state?
            if target_0 != target_1: return False

        return True
            

def do(SM, CreateNewStateMachineF=True):
    """Reduces the number of states according to equivalence classes of states. It starts
       with two sets: 
       
            (1) the set of acceptance states, 
                -- these states need to be splitted again according to their origin.
                   acceptance of state machine A is not equal to acceptance of 
                   state machine B.
            (2) the set of non-acceptance states.
       
       Whenever one finds a state in a state set that triggers on the same characters to 
       a different state set, the set has to be split in two sets of states:

           -- the set of states that trigger on trigger 'X' to state set 'K'
           -- the set of states that trigger on trigger 'X' to another state set

       The original state set is replaced by the two new ones. This algorithm is 
       repeated until the state sets do not change anymore.
    """        
    # (*) main algorithm    
    state_set_list = StateSet_List(SM)

    state_set_list_changed_f = True   
    while state_set_list_changed_f:
        # Loop over all sets in state set
        # by default the next state set list is the same
        i                        = 0              # -- loop index of the state set
        state_set_list_changed_f = False
        while i < state_set_list.size:
            if state_set_list.split(i):           
                state_set_list_changed_f = True   # -- a split happend, the state sets changed ...  
            i += 1

    # If all states in the state sets trigger equivalently, then the state set remains
    # nothing has to be done to the new state_set list, because its by default setup that way 
    if CreateNewStateMachineF: return create_state_machine(SM, state_set_list)
    else:                      return adapt_state_machine(SM, state_set_list)

def create_state_machine(SM, StateSetList):
    # If all states are of size one, this means, that there were no states that
    # could have been combined. In this case a simple copy of the original
    # state machine will do.
    if filter(lambda state_set: len(state_set) != 1, StateSetList.state_set_list) == []:
        return SM.clone()
    
    # Define a mapping from the state set to a new target state index
    map_new_state_index = {}
    for state_set_index in range(len(StateSetList.state_set_list)):
        map_new_state_index[state_set_index] = state_machine_index.get()
                
    # The state set that contains the initial state becomes the initial state of 
    # the new state machine.   
    state_set_containing_initial_state_i = StateSetList.map[SM.init_state_index]
    result = StateMachine(map_new_state_index[state_set_containing_initial_state_i],
                          Core = SM.core())

    # Ensure that each target state index has a state inside the state machine
    for new_state_index in map_new_state_index.values():
        result.create_new_state(StateIdx=new_state_index)

    # Build up the state machine out of the remaining state sets
    state_set_idx = -1L
    for state_set in StateSetList.state_set_list:
        state_set_idx += 1L
        assert len(state_set) != 0, "State set of size '0'. List = " + repr(StateSetList)

        # The prototype: States in one set behave all equivalent with respect to target state sets
        # thus only one state from the start set has to be considered.      
        prototype    = SM.states[state_set[0]]
        # The representive: shall represent the state set in the new state machine.
        representive = result.states[map_new_state_index[state_set_idx]]

        # The representive must have all transitions that the prototype has
        for target_state_index, trigger_set in prototype.transitions().get_map().items():
            target_state_set_index = StateSetList.map[target_state_index]
            representive.add_transition(trigger_set, 
                                        map_new_state_index[target_state_set_index])

        # Merge all core information of the states inside the state set.
        # If one state set contains an acceptance state, then the result is 'acceptance'.
        # (Note: The initial split separates acceptance states from those that are not
        #  acceptance states. There can be no state set containing acceptance and 
        #  non-acceptance states) 
        # (Note, that the prototype's info has not been included yet, consider whole set)
        for state_idx in state_set:
            representive.merge(SM.states[state_idx])

    return result    


def adapt_state_machine(sm, StateSetList):
    # If all states are of size one, this means, that there were no states that
    # could have been combined. In this case nothing is to be done.
    if filter(lambda state_set: len(state_set) != 1, StateSetList.state_set_list) == []:
        return sm
    
    # We know, that all states in a state set are equivalent. Thus, all but one
    # of each set can be thrown away.
    replacement_list = []
    for state_set in StateSetList.state_set_list:
        if len(state_set) == 1: continue

        # Merge all core information of the states inside the state set.
        prototype_index = state_set[0]
        prototype       = sm.states[state_set[0]]
        for state_idx in state_set[1:]:
            prototype.merge(sm.states[state_idx])

        # Throw the meaningless states away. Transitions to them need to 
        # point to the prototype
        for state_index in state_set[1:]:
            replacement_list.append([state_index, prototype_index])
            del sm.states[state_index]

    # Replace the indices of the thrown out states
    for x, y in replacement_list:
        for state in sm.states.values():
            state.transitions().replace_target_index(x, y)

    return sm    


