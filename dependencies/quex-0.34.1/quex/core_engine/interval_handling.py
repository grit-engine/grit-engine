#! /usr/bin/env python
# PURPOSE: Provides classes for handling of sets of numbers:
#
#     Interval: A continous set of numbers in a range from a
#              minimum to a maximum border.
# 
#     NumberSet: A non-continous set of numbers consisting of
#                described as a set of intervals.
#
# DATE: May 26, 2006
#
# (C) 2006 Frank-Rene Schaefer
#
# ABSOLUTELY NO WARRANTY
################################################################################

from copy import copy, deepcopy

import quex.core_engine.generator.languages.core as languages
import quex.core_engine.utf8                     as utf8
import sys

class Interval:
    """Representing an interval with a minimum and a maximum border. Implements
    basic operations on intervals: union, intersection, and difference.
    """
    def __init__(self, Begin=None, End=None):
        """NOTE: Begin = End signifies **empty** interval.

        Begin == None and End == None   => Empty Interval

        Begin == int and End == None    => Interval of size '1' (one number = Begin)
        
        Begin and End != None           => Interval starting from 'Begin' and the last
                                           element is 'End-1'

        """

        # .begin = smallest integer that belogns to interval.
        # .end   = first integer > 'Begin' that does **not belong** to interval
        if Begin == None and End == None:
            # empty interval
            self.begin = 0
            self.end   = 0
        else:
            if Begin == None:
                raise "Begin can only be 'None', if End is also 'None'!"
            self.begin = Begin            
            if End == None:  
                if self.begin != sys.maxint: self.end = self.begin + 1
                else:                        self.end = self.begin
            else:    
                self.end = End
            
    def is_empty(self):
        return self.begin == self.end

    def is_all(self):
        return self.begin == -sys.maxint and self.end == sys.maxint   
        print "##res:", result
 
    def contains(self, Number):
        """True  => if Number in NumberSet
           False => else
        """
        if Number >= self.begin and Number < self.end: return True
        else:                                          return False
        
    def check_overlap(self, Other):
        """Does interval overlap the Other?"""
        if self.begin  < Other.end and self.end > Other.begin: return True
        if Other.begin < self.end  and Other.end > self.begin: return True
        else:                                                  return False

    def check_touch(self, Other):
        """Does interval touch the Other?"""
        if self.begin  < Other.end and self.end > Other.begin:   return True
        if Other.begin < self.end  and Other.end > self.begin:   return True
        if self.begin == Other.begin or self.end == Other.begin: return True
        else:                                                    return False
    
    def union(self, Other):
        if self.check_overlap(Other):
            # overlap: return one single interval
            #          (the one that encloses both intervals)
            return [ Interval(min(self.begin, Other.begin),
                              max(self.end, Other.end)) ]
        else:
            # no overlap: two disjunct intervals
            result = []
            if not self.is_empty(): result.append(copy(self))
            if not Other.is_empty(): result.append(copy(Other))
            return result

    def intersection(self, Other):
        if self.check_overlap(Other):
            # overlap: return one single interval
            #          (the one that both have in common)
            return Interval(max(self.begin, Other.begin),
                            min(self.end, Other.end)) 
        else:
            # no overlap: empty interval (begin=end)
            return Interval()  # empty interval

    def difference(self, Other):
        """Difference self - Other."""
        if self.begin >= Other.begin:
            if self.end <= Other.end:
                # overlap: Other covers self
                # --------------[xxxxxxxxxxxxx]-------------------
                # ---------[yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy]-----
                #
                #          nothing remains - empty interval
                return []
            else:
                if self.begin >= Other.end:
                    # no intersection
                    return [ copy(self) ]
                else:
                    # overlap: Other covers lower part
                    # --------------[xxxxxxxxxxxxxxxxxxxx]------------
                    # ---------[yyyyyyyyyyyyyyyyyyyy]-----------------
                    return [ Interval(Other.end, self.end) ]
        else:            
            if self.end >= Other.end:
                # overlap: self covers Other
                # ---------[xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx]-----
                # --------------[yyyyyyyyyyyyy]-------------------
                #
                # result = two disjunct intervals
                result = []
                lower_part = Interval(self.begin, Other.begin)
                if not lower_part.is_empty(): result.append(lower_part)
                upper_part = Interval(Other.end, self.end)
                if not upper_part.is_empty(): result.append(upper_part)
                return result
            else:
                if self.end <= Other.begin:
                    # no intersection
                    return [ copy(self) ]
                else:
                    # overlap: Other upper part
                    # ---------[xxxxxxxxxxxxx]------------------------
                    # --------------[yyyyyyyyyyyyy]-------------------
                    #                
                    return [ Interval(self.begin, Other.begin) ]

    def inverse(self):
        if self.begin == self.end:
            # empty interval => whole range
            return [ Interval(-sys.maxint, sys.maxint) ]
        else:
            result = []
            if self.begin != -sys.maxint: result.append(Interval(-sys.maxint,self.begin))
            if self.end   != sys.maxint:  result.append(Interval(self.end, sys.maxint))
            return result

    def size(self):
        return self.end - self.begin

    def __repr__(self):
        return self.get_string(Option="")

    def __utf8_char(self, Code):
        if   Code == - sys.maxint:   return "-oo"
        elif Code == sys.maxint:     return "oo"            
        elif Code == ord(' '):       return "' '"
        elif Code == ord('\n'):      return "'\\n'"
        elif Code == ord('\t'):      return "'\\t'"
        elif Code == ord('\r'):      return "'\\r'"
        elif Code < ord(' '):        return "\\" + repr(Code) #  from ' ' to '9' things are 'visible'
        else:
            char_str = utf8.map_unicode_to_utf8(Code)
            return "'" + char_str + "'"
        # elif Code < ord('0') or Code > ord('z'): return "\\" + repr(Code)
        # else:                                    return "'" + chr(Code) + "'"

    def get_string(self, Option="", Delimiter=", "):
        if Option == "hex":    __repr = lambda x: "%05X" % x
        elif Option == "utf8": __repr = lambda x: self.__utf8_char(x)
        else:                  __repr = repr
        
        if self.begin == self.end:       return "[]"
        elif self.end - self.begin == 1: return "[" + __repr(self.begin) + "]" 
        else:                            return "[" + __repr(self.begin) + Delimiter + __repr(self.end-1) + "]"

    def get_utf8_string(self):
        #_____________________________________________________________________________
        assert self.begin <= self.end
        
        if self.begin == self.end: 
            return "''"
        elif self.end - self.begin == 1: 
            return self.__utf8_char(self.begin) 
        else:                          
            if   self.end == -sys.maxint: end_char = "-oo"
            elif self.end == sys.maxint:  end_char = "oo"
            else:                         end_char = self.__utf8_char(self.end-1)
            return "[" + self.__utf8_char(self.begin) + ", " + end_char + "]"

    def gnuplot_string(self, y_coordinate):
        if self.begin == self.end: return ""
        txt = ""
        txt += "%i %f\n" % (self.begin, y_coordinate)
        txt += "%i %f\n" % (self.end-1, y_coordinate)
        txt += "%i %f\n" % (self.end-1, float(y_coordinate) + 0.8)
        txt += "%i %f\n" % (self.begin, float(y_coordinate) + 0.8)
        txt += "%i %f\n" % (self.begin, y_coordinate)
        return txt

            
class NumberSet:
    """Represents an arbitrary set of numbers. The set is described
       in terms of intervals, i.e. objects of class 'Interval'. This
       class also provides basic operations such as union, intersection,
       and difference.
    """
    def __init__(self, Arg = None, ArgumentIsYoursF=False):
        """Arg = list     ==> list of initial intervals
           Arg = Interval ==> initial interval
           Arg = integer  ==> interval consisting of one number
           """
        arg_type = Arg.__class__
        assert arg_type in  [Interval, NumberSet, int, list] or Arg == None
        
        if  arg_type == list:
            if ArgumentIsYoursF:
                self.__intervals = Arg
            else:
                self.__intervals = []
                # use 'add_interval' to ensure consistency, i.e. touches, overlaps, etc.
                for interval in Arg:
                    self.add_interval(copy(interval))
            return

        if   arg_type == Interval:
            if ArgumentIsYoursF: self.__intervals = [ Arg ] 
            else:                self.__intervals = [ copy(Arg) ]

        elif arg_type == NumberSet:
            if ArgumentIsYoursF:  self.__intervals = Arg.__intervals
            else:                 self.__intervals = deepcopy(Arg.__intervals)

        elif arg_type == int:
            self.__intervals = [ Interval(Arg) ]

        else:
            self.__intervals = []

    def quick_append_interval(self, Other, SortF=True):
        """This function assumes that there are no intersections with other intervals.
           Use this function with caution. It is much faster than the 'union' function
           or the function 'add_interval'.
        """
        assert Other.__class__.__name__ == "Interval"

        self.__intervals.append(Other)

    def add_interval(self, NewInterval):
        """Adds an interval and ensures that no overlap with existing
        intervals occurs. Note: the 'touch' test is faster here, because
        only one interval is checked against.!"""
        if NewInterval.is_empty(): return
        
        # (1) determine if begin overlaps with the new interval
        if self.__intervals == [] or NewInterval.begin > self.__intervals[-1].end:
            self.__intervals.append(NewInterval)
            return

        X = NewInterval
        i = -1
        for y in self.__intervals:
            i += 1
            # possible cases:
            #  (1) [=== X ===]         [=== y ===]             
            #  
            #  (2) [=== X ============][=== y ===]
            #  
            #  (3) [=== X ==================]
            #                          [=== y ===]
            #  
            #  (4) [=== X =======================]
            #                          [=== y ===]
            #  
            #  (5) [=== X =============================================]
            #                          [=== y ===]
            #  
            #  (6)                     [=== X =========================]
            #                          [=== y ===]
            #  
            #  (7)                     [=== y ===][=== X ==============]
            #  
            #  (8)                     [=== y ===]           [=== X ===]
            #  
            if X.begin > y.end: 
                continue                              # filter (8)
            elif X.end < y.begin: 
                self.__intervals.insert(i, X) 
                return                                # filter (1)
            else:
                touch_begin = min(X.begin, y.begin) 
                break

        toucher_list    = [ ]
        insertion_index = i
        for y in self.__intervals[i:]:
            if X.end < y.begin: touch_end = X.end; break
            toucher_list.append(i)
            if X.end <= y.end: touch_end = y.end; break
            i += 1
        else:
            touch_end = X.end

        # (2) combine all intervals that intersect with the new one
        combination = Interval(touch_begin, touch_end)

        # (3) build new list of intervals
        #     (all overlaps are deleted, i.e. not added because they are
        #      replaced by the union with the NewInterval)
        # NOTE: The indices need to be adapted, since if for example
        #       interval '3' was an overlapper, and so '5' then if
        #       '3' is deleted, '5' comes at position '5'.
        offset = -1
        for i in toucher_list:
            offset += 1
            del self.__intervals[i - offset]

        self.__intervals.insert(insertion_index, combination)

    def cut_interval(self, CutInterval):
        """Adds an interval and ensures that no overlap with existing
        intervals occurs. Note: the 'touch' test is faster here, because
        only one interval is checked against.!"""
        assert CutInterval.__class__ == Interval
        if CutInterval.is_empty(): return
        
        # (*) determine if the interval has any intersection at all
        if    self.__intervals == []                              \
           or CutInterval.begin >  self.__intervals[-1].end       \
           or CutInterval.end   <= self.__intervals[0].begin:
            # (the cutting interval cannot cut out anything)
            return

        Y = CutInterval
        remainder_low = None
        remainder_up  = None
        # (*) find the first interval with which the cutting interval intersects.
        i = -1
        for x in self.__intervals:
            i += 1
            # (1) an intersecting interval is not yet reached
            if Y.begin >= x.end:    continue                        
            # (2) an intersecting interval was never reached
            #     (the cutting interval cannot cut out anything)
            elif Y.end < x.begin:   return
            # (3) INTERSECTION (implicit from above conditions)
            #     the following conditions are not mutually exclusive.
            #     from now on it is clear that the loop will be left.
            # (3a) the cut leaves a 'lower interval'
            if x.begin < Y.begin:   remainder_low = Interval(x.begin, Y.begin)
            # (3b) the cut leaves an 'upper interval'
            if x.end > Y.end:       remainder_up  = Interval(Y.end, x.end)
            # (3c) the interval has been swallowed completely 
            #      (both remainders stay empty)
            insertion_index = i
            break

        # (*) find the last interval that is concerned with the cut
        toucher_list = [ i ]

        if remainder_up == None and i != len(self.__intervals) - 1:
            for x in self.__intervals[i+1:]:
                i += 1
                # (1) last interval was swallowed complety, current interval has no intersection
                if Y.end <= x.begin: break
                # (2) INTERSECTION (implicit)
                toucher_list.append(i)
                # (2a) last intersecting interval (probably) not yet reached
                if Y.end > x.end:   continue
                # (2b) last cutting leaves an upper interval
                if Y.end < x.end:    
                    remainder_up  = Interval(Y.end, x.end)
                    break

        ## print "##", remainder_low
        ## print "##", remainder_up
        ## print "##", toucher_list

        # (*) build new list of intervals
        #     (all overlaps are deleted, i.e. not added because they are
        #      replaced by the union with the NewInterval)
        # NOTE: The indices need to be adapted, since if for example
        #       interval '3' was an overlapper, and so '5' then if
        #       '3' is deleted, '5' comes at position '5'.
        offset = -1
        for i in toucher_list:
            offset += 1
            del self.__intervals[i - offset]

        # insert the upper remainder first, so that it comes after the lower remainder
        if remainder_up  != None: self.__intervals.insert(insertion_index, remainder_up)
        if remainder_low != None: self.__intervals.insert(insertion_index, remainder_low)

    def contains(self, Number):
        """True  => if Number in NumberSet
           False => else
        """
        for interval in self.__intervals:
            if interval.contains(Number): return True
        return False

    def minimum(self):
        if self.__intervals == []: return sys.maxint   # i.e. an absurd value
        else:                      return self.__intervals[0].begin

    def supremum(self):
        if self.__intervals == []: return - sys.maxint # i.e. an absurd value
        else:                      return self.__intervals[0].end

    def is_empty(self):
        if self.__intervals == []: return True
        for interval in self.__intervals:
            if interval.is_empty() == False: return False
        return True
        
    def is_all(self):
        """Returns True if this NumberSet covers all numbers, False if not.
           
           Note: All intervals should have been added using the function 'add_interval'
                 Thus no overlapping intervals shall exists. If the set covers all numbers,
                 then there can only be one interval that 'is_all()'
        """
        if len(self.__intervals) != 1: return False
        return self.__intervals[0].is_all()
            
    def is_equal(self, Other):
        """Assume: All intervals are sorted and adjacent intervals are combined.
        """
        N = len(self.__intervals)
        if N != len(Other.__intervals): return False
        i = -1
        for interval in self.__intervals:
            i += 1
            other = Other.__intervals[i]
            if   interval.begin != other.begin: return False
            elif interval.end   != other.end:   return False
        return True

    def interval_number(self):
        """This value gives some information about the 'complexity' of the number set."""
        return len(self.__intervals)

    def get_intervals(self, PromiseNotToChangeAnythingF=False):
        if PromiseNotToChangeAnythingF: return self.__intervals
        else:                           return deepcopy(self.__intervals)

    def unite_with(self, Other):
        Other_type = Other.__class__
        assert Other_type == Interval or Other_type == NumberSet, \
               "Error, argument of type %s" % Other.__class__.__name__

        if Other_type == Interval:  
            self.add_interval(Other)
            return

        # simply add all intervals to one single set
        for interval in Other.__intervals:
            self.add_interval(interval)

    def union(self, Other):
        Other_type = Other.__class__
        assert Other_type == Interval or Other_type == NumberSet, \
               "Error, argument of type %s" % Other.__class__.__name__

        clone = deepcopy(self)

        if Other_type == Interval: 
            clone.add_interval(Other)
            return clone

        # simply add all intervals to one single set
        for interval in Other.__intervals:
            clone.add_interval(interval)

        return clone            

    def has_intersection(self, Other):
        assert Other.__class__ == Interval or Other.__class__ == NumberSet
        self_begin = self.__intervals[0].begin
        self_end   = self.__intervals[-1].end
        if Other.__class__ == Interval: 
            if Other.end   < self_begin: return False
            if Other.begin > self_end:   return False

            for y in self.__intervals:
                # PASTE: Implement Interval::overlap() for performance reasons.
                if   x.begin >= y.end:   continue
                elif x.end   <= y.begin: break
                # x.end > y.begin  (lacks condition: x.begin < y.end)
                # y.end > x.begin  (lacks condition: y.begin < x.end)
                if x.begin < y.end or y.begin < x.end: return True

            return False

        for x in Other.__intervals:
            if x.end < self_begin:   continue
            elif x.begin > self_end: break
            for y in self.__intervals:
                # PASTE: Implement Interval::overlap() for performance reasons.
                if   x.begin >= y.end:   continue
                elif x.end   <= y.begin: break
                # x.end > y.begin  (lacks condition: x.begin < y.end)
                # y.end > x.begin  (lacks condition: y.begin < x.end)
                if x.begin < y.end or y.begin < x.end: return True
        return False

    def has_only_this_element(self, Number):
        if   len(self.__intervals) != 1:              return False
        elif self.__intervals[0].begin != Number:     return False
        elif self.__intervals[0].end   != Number + 1: return False
        return True

    def intersect_with(self, Other):
        assert Other.__class__ == Interval or Other.__class__ == NumberSet

        if Other.__class__ == Interval: Other_intervals = [ Other ]
        else:                           Other_intervals = Other.__intervals
        
        if Other_intervals == [] or self.__intervals == []:     
            self.__intervals = []
            return 

        self_begin = self.__intervals[0].begin
        self_end   = self.__intervals[-1].end
        Other_begin = Other_intervals[0].begin
        Other_end   = Other_intervals[-1].end
        if Other_end  < self_begin or Other_begin > self_end:   
            self.__intervals = []
            return

        # For each interval to leave remain, it needs at least have an intersection
        # with one of the other intervals. If such an intersection is found the
        # interval of concern can be pruned appropriately.
        # print "##si0", self.__intervals
        L              = len(self.__intervals)
        insertion_list = []
        deletion_list  = []
        i              = -1
        begin_i        = -1
        end_i          = L
        for x in self.__intervals:
            i += 1
            if x.end   <= Other_begin: continue
            elif begin_i == -1:        begin_i = i; i -= begin_i; 
            if x.begin >= Other_end:   end_i   = i; break

            replacement_list = []
            for y in Other_intervals:
                if   x.begin >= y.end:   continue
                elif x.end   <= y.begin: break
                # x.end > y.begin  (lacks condition: x.begin < y.end)
                # y.end > x.begin  (lacks condition: y.begin < x.end)
                if x.begin < y.end or y.begin < x.end:
                    replacement_list.append([max(x.begin, y.begin), min(x.end, y.end)])

            if replacement_list != []:
                x.begin, x.end = replacement_list.pop(0)
                insertion_list.append([i, replacement_list])
            else:
                deletion_list.append(i)

        # -- delete the intervals that have no intersection
        if begin_i != -1: del self.__intervals[:begin_i]
        if end_i != L:    del self.__intervals[end_i:]
        offset = 0
        for i in deletion_list:
            del self.__intervals[i - offset]
            offset += 1

        # -- insert new intervals
        offset = 0
        for i, replacement_list in insertion_list:
            for begin, end in replacement_list:
                i += 1
                if i >= L: self.__intervals.append(Interval(begin, end))
                else:      self.__intervals.insert(i, Interval(begin, end))
            offset += i

    def intersection(self, Other):
        assert Other.__class__ == Interval or Other.__class__ == NumberSet

        if Other.__class__ == Interval: Other_intervals = [ Other ]
        else:                           Other_intervals = Other.__intervals

        # NOTE: If, for any reason this function does not rely on intersect_with(), then
        #       the function intersect_with() is no longer under unit test!
        result = deepcopy(self)
        result.intersect_with(Other)
        return result

    def subtract(self, Other):
        Other_type = Other.__class__
        assert Other_type == Interval or Other_type == NumberSet, \
               "Error, argument of type %s" % Other.__class__.__name__

        if Other_type == Interval:  
            self.cut_interval(Other)
            return

        Begin = self.__intervals[0].begin
        End   = self.__intervals[-1].end
        for interval in Other.__intervals:
            if interval.end   <= Begin: continue
            if interval.begin >= End:   break
            self.cut_interval(interval)

    def difference(self, Other):
        assert Other.__class__ == Interval or Other.__class__ == NumberSet

        clone = deepcopy(self)
        if Other.__class__ == Interval: 
            clone.cut_interval(Other)
            return clone

        Begin = self.__intervals[0].begin
        End   = self.__intervals[-1].end
        # note: there should be no internal overlaps according to 'add_interval'
        for interval in Other.__intervals:
            if interval.end   <= Begin: continue
            if interval.begin >= End:   break
            clone.cut_interval(interval)

        return clone

    def inverse(self):
        """Intersection of inverses of all intervals."""
        interval_list = []
        begin = - sys.maxint
        for interval in self.__intervals:
            interval_list.append(Interval(begin, interval.begin))
            begin = interval.end
        interval_list.append(Interval(begin, sys.maxint))

        return NumberSet(interval_list, ArgumentIsYoursF=True)
        
    def clean(self, SortF=True):
        """Combines adjacent and intersecting intervals to one.
        """

        # (2) Combine adjacent intervals
        L = len(self.__intervals)
        if L < 2: return

        new_intervals = []
        i = 0 
        current = self.__intervals[0]
        while i < L - 1:
            i += 1
            next = self.__intervals[i]

            if current.end < next.begin: 
                # (1) no intersection?
                new_intervals.append(current)
                current = next
            else:
                # (2) intersection:  [xxxxxxxxxxxxxxx]
                #                              [yyyyyyyyyyyyyyyyy]
                #          becomes   [xxxxxxxxxxxxxxxxxxxxxxxxxxx]
                #
                # => do not append interval i + 1, do not consider i + 1
                if current.end > next.end:
                    # no adaptions necessary, simply ignore next interval
                    pass
                else:
                    # adapt upper border of current interval to the end of the next
                    current.end = next.end

        new_intervals.append(current)


        self.__intervals = new_intervals

    def __repr__(self):
        return repr(self.__intervals)

    def get_utf8_string(self):
        msg = ""
        for interval in self.__intervals:
            msg += interval.get_utf8_string() + ", "
        if msg != "": msg = msg[:-2]
        return msg

    def gnuplot_string(self, y_coordinate):
        txt = ""
        for interval in self.__intervals:
            txt += interval.gnuplot_string(y_coordinate)
            txt += "\n"
        return txt

    def condition_code(self,
                       Language     = "C",
                       FunctionName = "example"):

        LanguageDB = languages.db[Language]
        txt  = LanguageDB["$function_def"].replace("$$function_name$$", FunctionName)
        txt += self.__condition_code(LanguageDB)
        txt += LanguageDB["$function_end"]

        return txt

    def __condition_code(self, LanguageDB,
                         LowestInterval_Idx = -1, UppestInterval_Idx = -1, 
                         NoIndentF = False):
        
        """Writes code that does a mapping according to 'binary search' by
        means of if-else-blocks.
        """
        if LowestInterval_Idx == -1 and UppestInterval_Idx == -1:
            LowestInterval_Idx = 0
            UppestInterval_Idx = len(self.__intervals) - 1
            
        if NoIndentF:
            txt = ""
        else:
            txt = "    "

        MiddleInterval_Idx = (UppestInterval_Idx + LowestInterval_Idx) / 2
        
        # quick check:
        assert UppestInterval_Idx >= LowestInterval_Idx, \
               "NumberSet::conditions_code(): strange interval indices:" + \
               "lowest interval index = " + repr(LowestInterval_Idx) + \
               "uppest interval index = " + repr(UppestInterval_Idx)
        
        middle = self.__intervals[MiddleInterval_Idx]
        
        if LowestInterval_Idx == UppestInterval_Idx \
           and middle.begin == middle.end - 1:
            # middle == one element
            txt += "$if %s $then\n" % LanguageDB["$=="]("input", repr(middle.begin))
            txt += "    $return_true\n"
            txt += "$endif"
            txt += "$return_false\n"
            
        else:
            # middle interval > one element
            txt += "$if input $>= %s $then\n" % repr(middle.end)

            if MiddleInterval_Idx == UppestInterval_Idx:
                # upper interval = none
                txt += "    $return_false\n"
                txt += "$endif"
            else:
                # upper intervals = some
                txt += self.__condition_code(LanguageDB,
                                             MiddleInterval_Idx + 1, UppestInterval_Idx)
                txt += "$endif"

            txt += "$if input $>= %s $then\n" % repr(middle.begin)
            txt += "    $return_true\n"
            txt += "$endif" 

            if MiddleInterval_Idx == LowestInterval_Idx:
                # lower intervals = none
                txt += "$return_false\n"
            else:
                # lower intervals = some
                txt += self.__condition_code(LanguageDB,
                                             LowestInterval_Idx, MiddleInterval_Idx - 1,
                                             NoIndentF = True)
            
        # return program text for given language
        return languages.replace_keywords(txt, LanguageDB, NoIndentF)

