from copy import deepcopy
from quex.core_engine.state_machine.state_core_info import StateCoreInfo

class StateOriginList:
    def __init__(self):
        self.__list = []

    def get_list(self):
        return self.__list

    def __add(self, Origin):
        """Check if origin has already been mentioned, else append the new origin.
        """
        if Origin in self.__list: 
            idx = self.__list.index(Origin)  
            self.__list[idx] = Origin
        else:    
            self.__list.append(Origin)

    def add(self, X, StateIndex, StoreInputPositionF=False, SelfAcceptanceF=False):
        """Add the StateMachineID and the given StateIdx to the list of origins of 
           this state.
           NOTE: The rule is that by default the 'store_input_position_f' flag
                 follows the acceptance state flag (i.e. by default any acceptance
                 state stores the input position). Thus when an origin is  added
                 to a state that is an acceptance state, the 'store_input_position_f'
                 has to be raised for all incoming origins.      
        """
        assert type(X) == long or X.__class__ == StateCoreInfo
        assert type(StateIndex) == long
            
        # -- entry checks 
        if X.__class__ == StateCoreInfo:
            self.__add(deepcopy(X))
        else:
            # -- create the origin data structure (X = state machine id)
            if StoreInputPositionF == None: StoreInputPositionF = SelfAcceptanceF
            self.__add(StateCoreInfo(StateMachineID      = X, 
                                     StateIndex          = StateIndex, 
                                     AcceptanceF         = SelfAcceptanceF,
                                     StoreInputPositionF = StoreInputPositionF))

    def append(self, OriginList, StoreInputPositionFollowsAcceptanceF, SelfAcceptanceF):
        """Add list of origins. Optional argument tells wether
           the 'store_input_position_f' shall adapt to the acceptance of self, or
           the acceptance of the origin list is to be copied.
        """
        if StoreInputPositionFollowsAcceptanceF: 
            for origin in OriginList:
                self.add(origin.state_machine_id, origin.state_index, 
                         StoreInputPositionF=self.is_acceptance())
        else:
            for origin in OriginList: self.__add(origin)

    def clear(self):
        self.__list = []

    def set(self, OriginList, ArgumentIsYoursF=False):
        assert type(OriginList) == list
        if ArgumentIsYoursF: self.__list = OriginList
        else:                self.__list = deepcopy(OriginList)

    def is_empty(self):
        return self.__list == []

    def is_from_single_state(self, StateMachineID, StateIdx):
        if len(self.__list) != 1:                             return False
        if self.__list[0].state_machine_id != StateMachineID: return False
        if self.__list[0].state_index != StateIdx:            return False
        return True

    def contains_post_context_flag(self):
        for origin in self.__list:
            if origin.post_context_id() != -1L: return True
        return False                                

    def contains_store_input_position(self):
        for origin in self.__list:
            if origin.store_input_position_f() == True: return True
        return False

    def contains_any_pre_context_dependency(self):
        for origin in self.__list:
            if origin.pre_context_id() != -1L:       return True
            if origin.pre_context_begin_of_line_f(): return True
        return False    

    def contains_pre_context_id(self):
        for origin in self.__list:
            if origin.pre_context_id() != -1L: return True
        return False    

    def contains_pre_context_begin_of_line(self):
        for origin in self.__list:
            if origin.pre_context_begin_of_line_f(): return True
        return False    

    def adapt(self, StateMachineID, StateIndex):
        """Adapts all origins so that their original state is 'StateIndex' in state machine
           'StateMachineID'. Post- and pre-condition flags remain, and so the store input 
           position flag.
        """
        for origin in self.__list:
            origin.state_machine_id = StateMachineID
            origin.state_index      = StateIndex 

    def find_first_acceptance_origin(self):
        """Returns first origin which refers to an acceptance state. 
           Note, that this only makes sense if the list is sorted.

           Returns 'None' if no acceptance origin has been found.
        """
        for origin in self.get_list():
            if origin.is_acceptance():
                return origin
        else:
            return None

    def delete_meaningless(self):
        """Deletes origins that are not concerned with one of the three:
           -- post-conditions
           -- pre-conditions/also trivials
           -- store input positions

           NOTE: This function is only to be used for single patterns not for
                 combined state machines. During the NFA to DFA translation
                 more than one state is combined into one. This maybe reflected
                 in the origin list. However, only at the point when the 
                 pattern state machine is ready, then the origin states are something
                 meaningful. The other information has to be kept.
                 
           NOTE: After applying this fuction to a single pattern, there should only
                 be one origin for each state.
        """
        self.__list = filter(lambda origin:
                                    origin.post_contexted_acceptance_f() or
                                    origin.pre_context_id() != -1L       or
                                    origin.store_input_position_f()        or
                                    origin.pre_context_begin_of_line_f(),
                                    self.__list)

    def delete_dominated(self):
        """This function is a simplification in order to allow the Hopcroft Minimization
           to be more efficient. It 'simulates' the code generation where the first unconditional
           pattern matches. The remaining origins of a state are redundant.

           This function is to be seen in analogy with the function 'get_acceptance_detector'. 
           Except for the fact that it requires the 'end of core pattern' markers of post
           conditioned patterns. If the markers are not set, the store input position commands
           are not called properly, and when restoring the input position bad bad things happen 
           ... i.e. segmentation faults.
        """
        # NOTE: Acceptance origins sort before non-acceptance origins
        self.__list.sort()
        new_origin_list = []
        unconditional_acceptance_found_f = False
        for origin in self.__list:

            if origin.is_acceptance():
                # Only append acceptance origins until the first unconditional acceptance state 
                # is found. 
                if not unconditional_acceptance_found_f:
                    if origin.pre_context_id() == -1L and not origin.pre_context_begin_of_line_f():
                        unconditional_acceptance_found_f = True # prevent entering this part again
                    new_origin_list.append(origin)

            else:
                # Non-Acceptance origins do not harm in any way. Actually, the origins
                # with 'origin.is_end_of_post_contexted_core_pattern() == True' **need**
                # to be in there. See the comment at the entry of this function.
                new_origin_list.append(origin)

        self.__list = new_origin_list 

    def get_string(self):
        txt = " <~ "
        if self.__list == []: return txt + "\n"
        for origin in self.__list:
            txt += repr(origin) + ", "
        txt = (txt[:-2] + "\n").replace("L","")     
        return txt
