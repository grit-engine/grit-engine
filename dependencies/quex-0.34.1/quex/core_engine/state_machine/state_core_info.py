
StateOriginInfo_POST_CONDITION_END          = 1
StateOriginInfo_NON_ACCEPTANCE              = 2
StateOriginInfo_ACCEPTANCE                  = 3
StateOriginInfo_POST_CONDITIONED_ACCEPTANCE = 4
StateOriginInfo_PRE_CONDITIONEND_ACCEPTANCE = 5
StateOriginInfo_ERROR                       = -1

class StateCoreInfo:
    """-- store input position: if an origin is flagged that way it 
          imposes that the input position is to be stored.

       -- post conditioned acceptance: 
          indicates that the original pattern was a 'post-conditioned'
          pattern. It follows:

       -- if store input position is required, then it has to be 
          stored in a dedicated variable, because, multiple 
          patterns may be post conditioned and only the one (if one)
          succeeds, has to store set the input position to the position
          where the first post conditioned acceptance occured.

       -- if store input position is not required, this implies that
          the original state was the 'final acceptance' state, i.e.
          the end of the post condition. no input position is to be
          stored in this case, because the position of the first
          success is to be restored.

       -- pre condition id:
          id of the state machine that constitutes the pre-condition. 
          if not pre-condition is set it is -1L. 

       -- pseudo ambiguous post condition id:
          id of the state machine that constitutes the state machine 
          that is used to go backwards to search the end of the core
          condition.
          In the large majority of cases, where there is no pseudo 
          ambiguous post condition (e.g. x*/x), it is set it is -1L. 
    """    
    def __init__(self, StateMachineID, StateIndex, AcceptanceF, StoreInputPositionF=False, 
                 PostContextID=-1L, PreContext_StateMachineID=-1L,
                 PreContext_BeginOfLineF=False,
                 PseudoAmbiguousPostConditionID=-1L):
        assert type(StateIndex) == long
        assert type(StateMachineID) == long

        self.state_machine_id = StateMachineID
        self.state_index      = StateIndex
        # is a acceptance state?
        self.__acceptance_f = AcceptanceF 

        # Input position of the current input stream is to be stored in 
        # the following cases: 
        #
        #   -- 'normal acceptance state', i.e. not the acceptance state of a post condition.
        #   -- 'ex acceptance state', i.e. states that were acceptance states of state machines
        #      that have a post condition.      
        #
        # NOTE: By default, the function 'set_acceptance(True)' and 'set_acceptance(False)'
        #       of class State sets the 'store_input_position_f' to True, respectively
        #       false, because this is the normal case. When a post condition is to be appended
        #       the 'store_input_position_f' is to be stored manually - see the function
        #       'state_machine.post_context_append.do(...).
        self.__store_input_position_f = StoreInputPositionF

        # -- was the origin a post-conditioned acceptance?
        #    (then we have to store the input position, store the original state machine
        #     as winner, but continue)
        self.__post_context_id = PostContextID

        # -- was the origin a pre-conditioned acceptance?
        #    (then one has to check at the end if the pre-condition holds)
        self.__pre_context_id = PreContext_StateMachineID  
        #
        # -- trivial pre-condition: begin of line
        self.__pre_context_begin_of_line_f = PreContext_BeginOfLineF

        # -- id of state machine that is used to go backwards from the end
        #    of a post condition that is pseudo-ambiguous. 
        self.__pseudo_ambiguous_post_context_id = PseudoAmbiguousPostConditionID

    def merge(self, Other):
        # It **does** make any sense to merge to state cores from different
        # state machines. This should not be an 'assert'. In the final state machine
        # more than one state machine is combined in parallel and then they belong 
        # to the same state machine
        # if self.state_machine_id != Other.state_machine_id: return

        if Other.__acceptance_f:                 self.__acceptance_f                = True
        if Other.__store_input_position_f:       self.__store_input_position_f      = True 
        if Other.__pre_context_begin_of_line_f:  self.__pre_context_begin_of_line_f = True 

        if Other.__pre_context_id != -1L:   self.__pre_context_id  = Other.__pre_context_id 
        if Other.__post_context_id != -1L:  self.__post_context_id = Other.__post_context_id

        if Other.__pseudo_ambiguous_post_context_id != -1L: 
            self.__pseudo_ambiguous_post_context_id = Other.__pseudo_ambiguous_post_context_id

    def is_acceptance(self):
        return self.__acceptance_f

    def set_acceptance_f(self, Value, LeaveStoreInputPositionF):
        """NOTE: By default, when a state is set to acceptance the input
                 position is to be stored for all related origins, if this is 
                 not desired (as by 'post_context_append.do(..)' the flag
                 'store_input_position_f' is to be adpated manually using the
                 function 'set_store_input_position_f'
        """      
        assert type(Value) == bool
        self.__acceptance_f = Value
        # default: store_input_position_f follows acceptance_f
        if not LeaveStoreInputPositionF: self.set_store_input_position_f(Value)
        
    def set_store_input_position_f(self, Value=True):
        assert type(Value) == bool
        self.__store_input_position_f = Value

    def set_pre_context_id(self, Value=True):
        assert type(Value) == long
        self.__pre_context_id = Value

    def set_pre_context_begin_of_line_f(self, Value=True):
        assert type(Value) == bool
        self.__pre_context_begin_of_line_f = Value

    def set_post_context_id(self, Value):
        assert type(Value) == long
        self.__post_context_id = Value

    def set_post_context_backward_detector_sm_id(self, Value):
        assert type(Value) == long
        self.__pseudo_ambiguous_post_context_id = Value

    def pre_context_id(self):
        return self.__pre_context_id  

    def post_context_id(self):
        return self.__post_context_id     

    def pre_context_begin_of_line_f(self):
        return self.__pre_context_begin_of_line_f

    def store_input_position_f(self):
        return self.__store_input_position_f    

    def pseudo_ambiguous_post_context_id(self):
        return self.__pseudo_ambiguous_post_context_id

    def is_end_of_post_contexted_core_pattern(self):
        return self.post_context_id() != -1L and self.store_input_position_f()
                            
    def type(self):
        Acc   = self.is_acceptance()
        Store = self.store_input_position_f()
        Post  = self.post_context_id()
        Pre   = self.pre_context_id() 
        if     Acc and     Store and not Post and not Pre: return StateOriginInfo_ACCEPTANCE
        if not Acc and not Store and not Post and not Pre: return StateOriginInfo_NON_ACCEPTANCE
        if     Acc and     Store and     Post and not Pre: return StateOriginInfo_POST_CONDITIONED_ACCEPTANCE
        if     Acc and not Store and     Post and not Pre: return StateOriginInfo_POST_CONDITION_END    
        if     Acc and     Store and not Post and     Pre: return StateOriginInfo_PRE_CONDITIONEND_ACCEPTANCE
        # the constitution of the state origin is not valid --> return error
        return StateOriginInfo_ERROR    

    def __cmp__(self, Other):
        if self.is_acceptance() == True  and Other.is_acceptance() == False: return -1
        if self.is_acceptance() == False and Other.is_acceptance() == True:  return 1

        # NOTE: The state machine ID directly corresponds to the 'position in the list'
        #       where the pattern was specified. Low ID == early specification.
        return cmp(self.state_machine_id, Other.state_machine_id)
            
    def __eq__(self, other):
        """Two origins are the same if they origin from the same state machine and 
           have the same state index. If they then differ in the 'store_input_position_f'
           there is a major error. It would mean that one StateOriginInfo states about the
           same original state something different than another StateOriginInfo.
        """
        result = self.state_machine_id == other.state_machine_id and \
                 self.state_index      == other.state_index                           

        if result == True: 
            assert self.__store_input_position_f == other.__store_input_position_f, \
                   "Two StateOriginInfo objects report about the same state different\n" \
                   "information about the input being stored or not.\n" \
                   "state machine id = " + repr(self.state_machine_id) + "\n" + \
                   "state index      = " + repr(self.state_index)
            assert self.__pre_context_id == other.__pre_context_id, \
                   "Two StateOriginInfo objects report about the same state different\n" \
                   "information about the pre-conditioned acceptance.\n" \
                   "state machine id = " + repr(self.state_machine_id) + "\n" + \
                   "state index      = " + repr(self.state_index)
            assert self.__post_context_id == other.__post_context_id, \
                   "Two StateOriginInfo objects report about the same state different\n" \
                   "information about the post-conditioned acceptance.\n" \
                   "state machine id = " + repr(self.state_machine_id) + "\n" + \
                   "state index      = " + repr(self.state_index)

        return result     

    def __repr__(self):
        return self.get_string()

    def get_string(self, StateMachineAndStateInfoF=True):
        appendix = ""

        # ONLY FOR TEST: state.core
        if False and not StateMachineAndStateInfoF:
            if self.__acceptance_f: return "*"
            else:                   return ""

        if StateMachineAndStateInfoF:
            if self.state_machine_id != -1L:
                appendix += ", " + repr(self.state_machine_id).replace("L", "")
            if self.state_index != -1L:
                appendix += ", " + repr(self.state_index).replace("L", "")
        if self.__acceptance_f:        
            appendix += ", A"
        if self.__store_input_position_f:        
            appendix += ", S"
        if self.__post_context_id != -1L:  # post context id determined 'register' where input position
            #                              # stored
            appendix += ", P" + repr(self.__post_context_id).replace("L", "")
        if self.__pre_context_id != -1L:            
            appendix += ", pre=" + repr(self.__pre_context_id).replace("L", "")
        if self.__pseudo_ambiguous_post_context_id != -1L:            
            appendix += ", papc=" + repr(self.__pseudo_ambiguous_post_context_id).replace("L", "")
        if self.__pre_context_begin_of_line_f:
            appendix += ", bol"
        if len(appendix) > 2: 
            appendix = appendix[2:]

        return "(%s)" % appendix

