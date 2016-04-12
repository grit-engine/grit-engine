import sys

from   quex.core_engine.interval_handling import NumberSet, Interval

# definitions for 'history items':
INTERVAL_BEGIN            = True
INTERVAL_END              = False
# real (uni-code) character codes start at zero, thus, it is safe to use 
# -7777 as a marker for undefined borders.
INTERVAL_UNDEFINED_BORDER = -7777

class TransitionMap:
    def __init__(self):
        self.__db = {}               # [target index] --> [trigger set that triggers to target]
        self.__epsilon_target_index_list = []

    def is_empty(self):
        return len(self.__db) == 0 and len(self.__epsilon_target_index_list) == 0

    def is_DFA_compliant(self):
        """Checks if the current state transitions are DFA compliant, i.e. it
           investigates if trigger sets pointing to different targets intersect.
           RETURN:  True  => OK
                    False => Same triggers point to different target. This cannot
                             be part of a deterministic finite automaton (DFA).
        """
        # DFA's do not have epsilon transitions
        if self.__epsilon_target_index_list != []: return False

        # check wether trigger sets intersect
        all_trigger_sets = NumberSet()
        for trigger_set in self.__db.values():
            if all_trigger_sets.has_intersection(trigger_set): 
                return False
            else:
                all_trigger_sets.unite_with(trigger_set)

        return True

    def add_epsilon_target_state(self, TargetStateIdx):
        if TargetStateIdx not in self.__epsilon_target_index_list:
            self.__epsilon_target_index_list.append(TargetStateIdx)

    def add_transition(self, Trigger, TargetStateIdx): 
        """Adds a transition according to trigger and target index.
           RETURNS: The target state index (may be created newly).
        """
        assert type(TargetStateIdx) == long or TargetStateIdx == None
        assert Trigger.__class__ in [int, long, list, Interval, NumberSet] or Trigger == None

        if Trigger == None: # This is a shorthand to trigger via the remaining triggers
            Trigger = self.get_trigger_set_union().inverse()
        elif type(Trigger) == long: Trigger = Interval(int(Trigger), int(Trigger+1))
        elif type(Trigger) == int:  Trigger = Interval(Trigger, Trigger+1)
        elif type(Trigger) == list: Trigger = NumberSet(Trigger, ArgumentIsYoursF=True)

        if Trigger.__class__ == Interval:  
            if self.__db.has_key(TargetStateIdx): 
                self.__db[TargetStateIdx].add_interval(Trigger)
            else:
                self.__db[TargetStateIdx] = NumberSet(Trigger, ArgumentIsYoursF=True)
        else:
            if self.__db.has_key(TargetStateIdx): 
                self.__db[TargetStateIdx].unite_with(Trigger)
            else:
                self.__db[TargetStateIdx] = Trigger

        return TargetStateIdx

    def delete_transitions_to_target(self, TargetIdx):
        """Returns all triggers that lead to target 'TargetIdx'. If a trigger 'None' is returned
           it means that the epsilon transition triggers to target state. If the TargetIndex is 
           omitted the set of all triggers, except the epsilon triggers, are returned.
        """
        if not self.__db.has_key(TargetIdx): return
        del self.__db[TargetIdx]

    def delete_epsilon_target_state(self, TargetStateIdx):

        if TargetStateIdx in self.__epsilon_target_index_list:
            del self.__epsilon_target_index_list[self.__epsilon_target_index_list.index(TargetStateIdx)]

    def delete_transitions_on_character_list(self, CharacterCodeList):

        for trigger_set in self.__db.values():
            for char_code in CharacterCodeList:
                if trigger_set.contains(char_code):
                    trigger_set.cut_interval(Interval(char_code, char_code+1))

        self.delete_transitions_on_empty_trigger_sets()

    def delete_transitions_on_empty_trigger_sets(self):

        for target_index, trigger_set in self.__db.items():
            if trigger_set.is_empty(): del self.__db[target_index]

    def get_trigger_set_union(self):
        result = NumberSet()
        for trigger_set in self.__db.values():
            result.unite_with(trigger_set)

        return result

    def get_epsilon_target_state_index_list(self):
        return self.__epsilon_target_index_list

    def get_non_epsilon_target_state_index_list(self):
        return self.__db.keys()

    def get_target_state_index_list(self):
        result = self.__db.keys()
        for index in self.__epsilon_target_index_list:
            result.append(index)
        return result

    def get_resulting_target_state_index(self, Trigger):
        """This function makes sense for DFA's"""
        for target_index, trigger_set in self.__db.items():
            if trigger_set.contains(Trigger): return target_index
        return None

    def get_resulting_target_state_index_list(self, Trigger):
        """NOTE: This function makes sense for NFA's"""
        result = []
        for target_index, trigger_set in self.__db.items():
            if trigger_set.contains(Trigger) and target_index not in result:
                result.append(target_index) 

        if self.__epsilon_target_index_list != []:
            for target_index in self.__epsilon_target_index_list:
                if target_index not in result:
                    result.append(self.__epsilon_target_index_list)

        return result_list

    def get_map(self):
        return self.__db

    def get_trigger_set_line_up(self):
        ## WATCH AND SEE THAT WE COULD CACHE HERE AND GAIN A LOT OF SPEED during construction
        ## if self.__dict__.has_key("NONSENSE"): 
        ##    self.NONSENSE += 1
        ##    print "## called", self.NONSENSE
        ## else:
        ##    self.NONSENSE = 1
        """Lines the triggers up on a 'time line'. A target is triggered by
           certain characters that belong to 'set' (trigger sets). Those sets
           are imagined as ranges on a time line. The 'history' is described
           by means of history items. Each item tells wether a range begins
           or ends, and what target state is reached in that range.

           [0, 10] [90, 95] [100, 200] ---> TargetState0
           [20, 89]                    ---> TargetState1
           [96, 99] [201, 205]         ---> TargetState2

           results in a 'history':

           0:  begin of TargetState0
           10: end of   TargetState0
           11: begin of DropOut
           20: begin of TargetState1
           89: end of   TargetState1
           90: begin of TargetState0
           95: end of   TargetState0
           96: begin of TargetState2
           99: end of   TargetState2
           100 ...
           
        """
        # (*) create a 'history', i.e. note down any change on the trigger set combination
        #     (here the alphabet plays the role of a time scale)
        class history_item:
            def __init__(self, Position, ChangeF, TargetIdx):
                self.position   = Position
                self.change     = ChangeF
                self.target_idx = TargetIdx 
                
            def __repr__(self):         
                if self.change == INTERVAL_BEGIN: ChangeStr = "begin"
                else:                             ChangeStr = "end"
                return "%i: %s %s" % (self.position, ChangeStr, self.target_idx)

            def __eq__(self, Other):
                return     self.position   == Other.position \
                       and self.change     == Other.change   \
                       and self.target_idx == Other.target_idx 
                
        history = []
        # NOTE: This function only deals with non-epsilon triggers. Empty
        #       ranges in 'history' are dealt with in '.get_trigger_map()'. 
        for target_idx, trigger_set in self.__db.items():
            interval_list = trigger_set.get_intervals(PromiseNotToChangeAnythingF=True)
            for interval in interval_list: 
                # add information about start and end of current interval
                history.append(history_item(interval.begin, INTERVAL_BEGIN, target_idx))
                history.append(history_item(interval.end, INTERVAL_END, target_idx))

        # (*) sort history according to position
        history.sort(lambda a, b: cmp(a.position, b.position))

        return history      

    def get_trigger_map(self):
        """Consider the set of possible characters as aligned on a 1 dimensional line.
           This one-dimensional line is remiscent of a 'time line' so we call the change
           of interval coverage 'history'.         

           Returns a trigger map consisting of a list of pairs: (Interval, TargetIdx)

                    [ [ interval_0, target_0],
                      [ interval_1, target_1],
                      ...
                      [ interval_N, target_N] ]

           The intervals are sorted and non-overlapping (use this function only for DFA).

           A drop out on 'interval_i' is represented by 'target_i' == None.
        """
        # At this point only DFAs shall be considered. Thus there cannot be any epsilon
        # target transitions.
        assert self.__epsilon_target_index_list == [], \
               "Trigger maps can only be computed on DFAs. Epsilon transition detected."

        # NOTE: The response '[]' is a **signal** that there is only an epsilon
        #       transition. The response as such would be incorrect. But the signal
        #       'empty reply' needs to be treated by the caller.
        if len(self.__db) == 0: return []
            
        history = self.get_trigger_set_line_up()
        
        def query_trigger_map(EndPosition, TargetIdx):
            """Find all map entries that have or have not an open interval and
               point to the given TargetIdx. If TargetIdx = None it is not checked.
            """
            entry_list = []
            for entry in trigger_map:
                if entry[0].end == EndPosition and entry[1] == TargetIdx: 
                    entry_list.append(entry)
            return entry_list 

        # (*) build the trigger map
        trigger_map = []    
        for item in history:
            if item.change == INTERVAL_BEGIN: 
                # if an interval has same target index and ended at the current
                # intervals begin, then extend the last interval, do not create a new one.
                adjacent_trigger_list = query_trigger_map(item.position, item.target_idx)
                if adjacent_trigger_list == []:
                    # open a new interval (set .end = None to indicate 'open')
                    trigger_map.append([Interval(item.position, INTERVAL_UNDEFINED_BORDER), 
                                       item.target_idx])
                else:
                    for entry in adjacent_trigger_list: 
                        # re-open the adjacent interval (set .end = None to indicate 'open')
                        entry[0].end = INTERVAL_UNDEFINED_BORDER
                
            else:
                # item.change == INTERVAL_END   
                # close the correspondent intervals
                # (search for .end = None indicating 'open interval')           
                for entry in query_trigger_map(INTERVAL_UNDEFINED_BORDER, item.target_idx):
                    entry[0].end = item.position
            
        # (*) fill all gaps in the trigger map with 'None' target = Drop Out !

        drop_out_target = None
        ## NOTE: Trigger maps shall only be computed for DFA, see assert above. 
        #        Thus, there cannot be an epsilon transition.
        ## if self.__drop_out_target_index_list != []: 
        ##    drop_out_target = self.__drop_out_target_index_list[0]   
        gap_filler = [] 
        if len(trigger_map) >= 2:    
            prev_entry = trigger_map[0]    
            for entry in trigger_map[1:]:    
                if prev_entry[0].end != entry[0].begin: 
                    gap_filler.append([Interval(prev_entry[0].end, entry[0].begin), drop_out_target])
                prev_entry = entry
    
        # -- append the last interval until 'infinity' (if it is not yet specified   
        #    (do not switch this with the following step, .. or you screw it)           
        if trigger_map[-1][0].end != sys.maxint:    
            trigger_map.append([Interval(trigger_map[-1][0].end, sys.maxint), drop_out_target])
        # -- insert a first interval from -'infinity' to the start of the first interval
        if trigger_map[0][0].begin != -sys.maxint:
            trigger_map.append([Interval(-sys.maxint, trigger_map[0][0].begin), drop_out_target])
           
        # -- insert the gap fillers and get the trigger map straigtened up again
        trigger_map.extend(gap_filler) 
        trigger_map.sort(lambda a, b: cmp(a[0].begin, b[0].begin))

        # (*) post check assert
        for entry in trigger_map:
            assert entry[0].end != None, \
                   "remaining open intervals in trigger map construction"

        return trigger_map

    def get_trigger_set_to_target(self, TargetIdx):
        """Returns all triggers that lead to target 'TargetIdx'. If a trigger 'None' is returned
           it means that the epsilon transition triggers to target state. If the TargetIndex is 
           omitted the set of all triggers, except the epsilon triggers, are returned.
        """
        if self.__db.has_key(TargetIdx): return self.__db[TargetIdx]
        else:                            return NumberSet()

    def replace_target_index(self, Before, After):
        """Replaces given target index 'Before' with the index 'After'. 
           This means, that a transition targetting to 'Before' will then transit
           to 'After'.
        """   
        # replace target index in the 'normal map'
        if not self.__db.has_key(Before): 
            pass
        elif self.__db.has_key(After):    
            self.__db[After].unite_with(self.__db[Before])
            del self.__db[Before]
        else: 
            self.__db[After] = self.__db[Before]
            del self.__db[Before]

        # replace target index in the list of epsilon transition targets.
        if Before in self.__epsilon_target_index_list:
            self.__epsilon_target_index_list[self.__epsilon_target_index_list.index(Before)] = After

    def replace_drop_out_target_states_with_adjacent_targets(self):
        # NOTE: The request does not invalidate anything, invalidate cache after that.
        trigger_map = self.get_trigger_map() 


        if trigger_map == []:  # Nothing to be done, since there is nothing adjacent 
            return             # to the 'drop out' trigger. There is only an epsilon transition.

        assert len(trigger_map) >= 2

        # Target of internval (-oo, X) must be 'drop out' since there are no unicode 
        # code points below 0.
        assert trigger_map[0][1] == None
        assert trigger_map[0][0].begin == - sys.maxint


        # The first interval mentioned after that must not point to 'drop out' since
        # the trigger map must collect the same targets into one single interval.
        assert trigger_map[1][1] != None

        non_drop_out_target = trigger_map[1][1]
        self.add_transition(trigger_map[0][0], non_drop_out_target)
        
        # NOTE: Here we know that len(trigger_map) >= 2
        for trigger_set, target in trigger_map[2:]:

            if target == None: target = non_drop_out_target
            else:              non_drop_out_target = target

            self.add_transition(trigger_set, target)

    def has_one_of_triggers(self, CharacterCodeList):
        assert type(CharacterCodeList) == list
        for code in CharacterCodeList:
            if self.has_trigger(code): return True
        return False

    def has_trigger(self, CharCode):
        assert type(CharCode) == int
        if self.get_resulting_target_state_index(CharCode) == None: return False
        else:                                                       return True

    def get_string(self, FillStr, StateIndexMap):
        # print out transitionts
        sorted_transitions = self.get_map().items()
        sorted_transitions.sort(lambda a, b: cmp(a[1].minimum(), b[1].minimum()))

        msg = ""
        # normal state transitions
        for target_state_index, trigger_set in sorted_transitions:
            trigger_str = trigger_set.get_utf8_string()
            if StateIndexMap == None: target_str = "%05i" % target_state_index
            else:                     target_str = "%05i" % StateIndexMap[target_state_index]
                
            msg += "%s == %s ==> %s\n" % (FillStr, trigger_str, target_str)

        # epsilon transitions
        if self.__epsilon_target_index_list != []:
            txt_list = map(lambda ti: "%05i" % StateIndexMap[ti], self.__epsilon_target_index_list)
            msg += "%s ==<epsilon>==> " % FillStr 
            for txt in txt_list:
                msg += txt + ", "
            if len(txt_list) != 0: msg = msg[:-2]
        else:
            msg += "%s <no epsilon>" % FillStr

        msg += "\n"

        return msg

    def get_graphviz_string(self, OwnStateIdx, StateIndexMap):
        sorted_transitions = self.get_map().items()
        sorted_transitions.sort(lambda a, b: cmp(a[1].minimum(), b[1].minimum()))

        msg = ""
        # normal state transitions
        for target_state_index, trigger_set in sorted_transitions:
            trigger_str = trigger_set.get_utf8_string()
            if StateIndexMap == None: target_str  = "%i" % target_state_index
            else:                     target_str  = "%i" % StateIndexMap[target_state_index]
            msg += "%i -> %s [label =\"%s\"];\n" % (OwnStateIdx, target_str, trigger_str.replace("\"", ""))

        # epsilon transitions
        for ti in self.__epsilon_target_index_list:
            if StateIndexMap == None: target_str = "%i" % int(ti) 
            else:                     target_str = "%i" % int(StateIndexMap[ti]) 
            msg += "%i -> %s [label =\"<epsilon>\"];\n" % (OwnStateIdx, target_str)

        return msg

