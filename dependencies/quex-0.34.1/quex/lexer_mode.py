################################################################################
#! /usr/bin/env python
# Quex is  free software;  you can  redistribute it and/or  modify it  under the
# terms  of the  GNU Lesser  General  Public License  as published  by the  Free
# Software Foundation;  either version 2.1 of  the License, or  (at your option)
# any later version.
# 
# This software is  distributed in the hope that it will  be useful, but WITHOUT
# ANY WARRANTY; without even the  implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the  GNU Lesser General Public License for more
# details.
# 
# You should have received a copy of the GNU Lesser General Public License along
# with this  library; if not,  write to the  Free Software Foundation,  Inc., 59
# Temple Place, Suite 330, Boston, MA 02111-1307 USA
#
# (C) 2006, 2007 Frank-Rene Schaefer
#
################################################################################
import sys
import os

from copy                import copy
from quex.frs_py.file_in import error_msg
from quex.core_engine.generator.action_info import *

#-----------------------------------------------------------------------------------------
# mode_db: storing the mode information into a dictionary:
#          key  = mode name
#          item = LexMode object
#-----------------------------------------------------------------------------------------
mode_db = {}


class OptionInfo:
    """This type is used only in context of a dictionary, the key
       to the dictionary is the option's name."""
    def __init__(self, Type, Domain=-1):
        # self.name = Option see comment above
        self.type   = Type
        if Type == "list" and Domain == -1: self.domain = []
        else:                               self.domain = Domain

class LexMode:
    def __init__(self, Name, Filename, LineN):
        global mode_db

        self.filename = Filename
        self.line_n   = LineN

        self.name       = Name
        self.base_modes = []
        # Read pattern information into dictionary object. This allows for the following:
        # (i)   inheritance of pattern behavior in different modes.
        # (ii)  'virtual' patterns in the sense that their behavior can be
        #       overwritten.
        self.__matches = {}  # genuine patterns as specified in the mode declaration

        self.__repriorization_db = {}  # patterns of the base class to be reprioritized
        #                              # map: pattern --> new pattern index
        self.__deletion_db       = {}  # patterns of the base class to be deleted

        self.__inheritance_resolved_f = False # inherited patterns are only valid, after the
        #                                     # function 'pattern_action_pairs()' has been
        #                                     # called, since it resolves all inheritance issues.
        self.__resolved_matches = {}          # own and inherited patterns after call to function
        #                                     # 'pattern_action_pairs().

        # register at the mode database
        mode_db[Name] = self

        # (*) set the information about possible options
        self.options = {}       # option settings

        # -- default settings for options
        self.options["inheritable"]        = "yes"
        self.options["exit"]               = []
        self.options["entry"]              = []
        self.options["restrict"]           = []
        self.options["skip"]               = []
        self.options["skip-range"]         = []
        self.options["skip-nesting-range"] = []

        self.on_entry         = CodeFragment()
        self.on_exit          = CodeFragment()
        self.on_match         = CodeFragment()
        self.on_failure       = CodeFragment()
        self.on_end_of_stream = CodeFragment()
        self.on_indentation   = CodeFragment()

        # A flag indicating wether the mode has gone trough
        # consistency check.
        self.consistency_check_done_f             = False
        self.inheritance_circularity_check_done_f = False

    def has_event_handler(self):
        if   self.on_entry.get_code()         != "": return True
        elif self.on_exit.get_code()          != "": return True
        elif self.on_match.get_code()         != "": return True
        elif self.on_failure.get_code()       != "": return True
        elif self.on_end_of_stream.get_code() != "": return True
        elif self.on_indentation.get_code()   != "": return True
        else:                                        return False

    def has_pattern(self, PatternStr):
        return self.__matches.has_key(PatternStr)

    def get_match_object(self, PatternStr):
        return self.__matches[PatternStr]

    def has_matches(self):
        assert self.inheritance_circularity_check_done_f == True, \
               "called before consistency check!"

        if self.__matches != {}: return True

        for name in self.base_modes:
           if mode_db[name].has_matches(): return True

        return False

    def own_matches(self):
        return self.__matches

    def add_match(self, Pattern, Action, PatternStateMachine):
        if self.__matches.has_key(Pattern):
            error_msg("Pattern '%s' appeared twice in mode definition.\n" % Pattern + \
                      "Only the last definition is considered.", 
                      Action.filename, Action.line_n, DontExitF=True)

        self.__matches[Pattern] = PatternActionInfo(PatternStateMachine, Action, Pattern)

    def add_match_priority(self, Pattern, PatternStateMachine, PatternIdx, fh):
        if self.__matches.has_key(Pattern):
            error_msg("Pattern '%s' appeared twice in mode definition.\n" % Pattern + \
                      "Only this priority mark is considered.", fh)

        self.__repriorization_db[Pattern] = PatternIdx

    def add_match_deletion(self, Pattern, PatternStateMachine, fh):
        if self.__matches.has_key(Pattern):
            error_msg("Deletion of '%s' which appeared before in same mode.\n" % Pattern + \
                      "Deletion of pattern.", fh)

        self.__deletion_db[Pattern] = True

    def on_entry_code_fragments(self, Depth=0):
        """Collect all 'on_entry' event handlers from all base classes.
           Returns list of 'CodeFragment'.
        """
        return self.__collect_fragments("on_entry")

    def on_exit_code_fragments(self, Depth=0):
        """Collect all 'on_exit' event handlers from all base classes.
           Returns list of 'CodeFragment'.
        """
        return self.__collect_fragments("on_exit")

    def on_indentation_code_fragments(self, Depth=0):
        """Collect all 'on_indentation' event handlers from all base classes.
           Returns list of 'CodeFragment'.
        """
        return self.__collect_fragments("on_indentation")

    def on_match_code_fragments(self, Depth=0):
        """Collect all 'on_match' event handlers from all base classes.
           Returns list of 'CodeFragment'.
        """
        return self.__collect_fragments("on_match")

    def on_end_of_stream_code_fragments(self, Depth=0):
        """Collect all 'on_end_of_stream' event handlers from all base classes.
           Returns list of 'CodeFragment'.
        """
        return self.__collect_fragments("on_end_of_stream")

    def on_failure_code_fragments(self, Depth=0):
        """Collect all 'on_failure' event handlers from all base classes.
           Returns list of 'CodeFragment'.
        """
        return self.__collect_fragments("on_failure")

    def __collect_fragments(self, FragmentName, Depth=0):
        """Collect all event handlers with the given FragmentName from all base classes.
           Returns list of 'CodeFragment'.
        """
        if self.__dict__[FragmentName].get_code() == "":
            code_fragments = []
        else:      
            code_fragments = [ self.__dict__[FragmentName] ]

        for base_mode_name in self.base_modes:
            # -- get 'on_match' handler from the base mode
            base_mode = mode_db[base_mode_name]
            code_fragments.extend(base_mode.on_match_code_fragments(Depth+1))

        # reverse the order, so that the lowest base class appears first
        if Depth==0: code_fragments.reverse()

        return code_fragments

    def pattern_action_pairs(self, Depth=0):
        """Collect patterns of all inherited modes. Patterns are like virtual functions
        in C++ or other object oriented programming languages. Also, the patterns of the
        uppest mode has the highest priority, i.e. comes first.

        The list 'DerivedModes' is used in recursive calls (see '__collect_patterns') to
        determine a circular inheritance error.
        """

        # (1) collect patterns from base modes
        inherited_matches = {}
        for base_mode_name in self.base_modes:

            # -- get patterns of the base mode (the mode one inherits from)
            base_mode     = mode_db[base_mode_name]
            base_patterns = base_mode.pattern_action_pairs(Depth + 1)

            for pattern, match in base_patterns.items():
                inherited_matches[pattern] = match

        # (2) -- treat the overwriting of virtual patterns and
        #     -- reset the 'pattern index' and the 'inheritance level index'
        #        if a PRIORITY-MARK was set somewhere
        #     -- delete inherited patterns that are marked for deletion.
        resolved_matches = {}

        # -- loop over inherited patterns and add them to the list
        for inherited_pattern, inherited_match in inherited_matches.items():
            if   self.__deletion_db.has_key(inherited_pattern):
                # totally ignore the pattern, do not absorb it into 'resolved matches'
                pass
            elif self.__repriorization_db.has_key(inherited_pattern):
                # adapt the pattern index, this automatically adapts the priorization
                resolved_matches[inherited_pattern] = copy(inherited_match)
                new_id = self.__repriorization_db[inherited_pattern]
                resolved_matches[inherited_pattern].pattern_state_machine().core().set_id(new_id)
                resolved_matches[inherited_pattern].inheritance_level = Depth
                resolved_matches[inherited_pattern].inheritance_mode_name = self.name
            elif inherited_pattern not in self.__matches.keys():
                # inherited pattern did not at all occur in the own matches list
                resolved_matches[inherited_pattern] = copy(inherited_match)
            else:
                # own match overides the inherited pattern
                resolved_matches[inherited_pattern] = copy(inherited_match)
                resolved_matches[inherited_pattern].inheritance_level     = Depth
                resolved_matches[inherited_pattern].inheritance_mode_name = self.name

        # -- loop over own patterns and add what is not added yet
        #    (the priority marked patterns should now all be added. if
        #     a priority marked pattern in not present yet, it means that
        #     it was not mentioned in a base mode)
        for pattern, match in self.__matches.items():
            if resolved_matches.has_key(pattern): continue
            resolved_matches[pattern] = copy(match)
            resolved_matches[pattern].inheritance_level = Depth
            resolved_matches[pattern].inheritance_mode_name = self.name

        return resolved_matches

    def inheritance_structure_string(self, Depth=0):
        """NOTE: The consistency check ensures that all base modes are
                 defined and inheritable and there is no circular inheritance !
                 Therefore there is no need to check if the base mode
                 has an entry in the mode database."""
        assert self.consistency_check_done_f == True, \
               "called before consistency check!"

        if Depth != 0: str = "** " + ("   " * Depth) + self.name + "\n"
        else:          str = "** <" + self.name + ">\n"
        for base_mode_name in self.base_modes:
            mode = mode_db[base_mode_name]
            str += mode.inheritance_structure_string(Depth + 1)
        return str

    def get_base_modes(self):
        """Get all base classes recursively.
           NOTE: Circularity check needs to happen somewhere else
           This function is part of the consistency check!
        """
        assert self.inheritance_circularity_check_done_f == True, \
               "called before consistency check!"

        base_mode_collection = copy(self.base_modes)
        for base_mode in self.base_modes:
            # -- append base_mode to the base modes of current mode
            base_mode_collection_candidates = mode_db[base_mode].get_base_modes()
            for candidate in base_mode_collection_candidates:
                if candidate not in base_mode_collection:
                    base_mode_collection.append(candidate)

        return base_mode_collection

    def add_option(self, Option, Value):
        """ SANITY CHECK:
                -- which options are concatinated to a list
                -- which ones are replaced
                -- what are the values of the options
        """
        assert mode_option_info_db.has_key(Option)

        oi = mode_option_info_db[Option]
        if oi.type == "list":
            # append the value, assume in lists everything is allowed
            if self.options.has_key(Option): self.options[Option].append(Value)
            else:                            self.options[Option] = [ Value ]
        else:
            assert Value in oi.domain
            self.options[Option] = Value

    def consistency_check(self):
        """Checks for the following:

        -- existence of base modes
        -- circularity of inheritance
        -- existence of enter and exit modes
        -- checks that if X has an exit to Y,
           then Y must have an entry to X

        Any failure causes an immediate break up.
        """

        # (*) Base Modes
        #
        #   -- ability to inherit
        #
        #   NOTE: existence of modes is checked in ciruclarity test.
        #
        for base_mode_name in self.base_modes:
            # -- is base mode inheritable?
            if mode_db[base_mode_name].options["inheritable"] == "no":
                error_msg("mode '%s' inherits mode '%s' which is **not inheritable**." % \
                          (self.name, base_mode_name), self.filename, self.line_n)

        # -- require all bases modes
        all_base_modes = self.get_base_modes()

        # (*) A mode that does not force to be inherited needs finally contain matches.
        #     A mode that contains only event handlers is not <inheritable: only>, but
        #     somehow, it needs some matches in the base classes, otherwise it cannot
        #     act as a pattern state machine.
        if self.options["inheritable"] != "only" and self.has_matches() == False:
            error_msg("mode '%s' was allowed without <inheritable: only> despite it contains no matches\n" % \
                      (self.name) + \
                      "because it contains event handlers. Finally, though, it seems not want to inherit\n" + \
                      "any mode that contains matches. Therefore, it cannot act as a pattern detecting\n" + \
                      "state machine and cannot be a lexical analyzer mode.",
                      self.filename, self.line_n)

        # (*) Enter/Exit Transitions
        for mode_name in self.options["exit"]:
            # -- does other mode exist?
            if mode_db.has_key(mode_name) == False:
                error_msg("mode '%s'\nhas  an exit to mode '%s'\nbut no such mode exists." % \
                          (self.name, mode_name), self.filename, self.line_n)

            # -- does other mode have an entry for this mode?
            #    (does this or any of the base modes have an entry to the other mode?)
            other_mode = mode_db[mode_name]
            #    -- no restrictions => OK
            if other_mode.options["entry"] == []: continue
            #    -- restricted entry => check if this mode ore one of the base modes can enter
            for base_mode in [self.name] + all_base_modes:
                if base_mode in other_mode.options["entry"]: break
            else:
                error_msg("mode '%s'\nhas an exit to mode '%s'," % (self.name, mode_name),
                          self.filename, self.line_n, DontExitF=True)
                error_msg("but mode '%s'\nhas no entry for mode '%s'.\n" % (mode_name, self.name) + \
                          "or any of its base modes.",
                          other_mode.filename, other_mode.line_n)

        for mode_name in self.options["entry"]:
            # -- does other mode exist?
            if mode_db.has_key(mode_name) == False:
                error_msg("mode '%s'\nhas an entry from mode '%s'\nbut no such mode exists." % \
                          (self.name, mode_name),
                          self.filename, self.line_n)

            # -- does other mode have an exit for this mode?
            #    (does this or any of the base modes have an exit to the other mode?)
            other_mode = mode_db[mode_name]
            #    -- no restrictions => OK
            if other_mode.options["exit"] == []: continue
            #    -- restricted exit => check if this mode ore one of the base modes can enter
            for base_mode in [self.name] + all_base_modes:
                if base_mode in other_mode.options["exit"]: break
            else:
                error_msg("mode '%s'\nhas an entry for mode '%s'" % (self.name, mode_name),
                          self.filename, self.line_n, DontExitF=True)
                error_msg("but mode '%s'\nhas no exit to mode '%s'\n" % (mode_name, self.name) + \
                          "or any of its base modes.",
                          other_mode.filename, other_mode.line_n)
                
        self.consistency_check_done_f = True

    #def get_number_of_post_conditioned_patterns(self):
    #    """NOTE: The number of post conditions determines the size of an array. However,
    #             the analyser, later one determines it again and then it is correct. But,
    #             still keep in mind that this number is 'crucial'!
    #    """
    #    pattern_action_pairs = self.pattern_action_pairs()
    #    post_condition_n = 0
    #    for match in pattern_action_pairs.values():
    #        if match.pattern_state_machine.core().post_context_id() != -1L:
    #            post_condition_n += 1
    #    return post_condition_n

#-----------------------------------------------------------------------------------------
# mode option information/format: 
#-----------------------------------------------------------------------------------------
mode_option_info_db = {
   # -- a mode can be inheritable or not or only inheritable. if a mode
   #    is only inheritable it is not printed on its on, only as a base
   #    mode for another mode. default is 'yes'
   "inheritable": OptionInfo("single", ["no", "yes", "only"]),
   # -- a mode can restrict the possible modes to exit to. this for the
   #    sake of clarity. if no exit is explicitly mentioned all modes are
   #    possible. if it is tried to transit to a mode which is not in
   #    the list of explicitly stated exits, an error occurs.
   #    entrys work respectively.
   "exit":        OptionInfo("list"),
   "entry":       OptionInfo("list"),
   # -- a mode can restrict the exits and entrys explicitly mentioned
   #    then, a derived mode cannot add now exits or entrys
   "restrict":          OptionInfo("list", ["exit", "entry"]),
   # -- a mode can have 'skippers' that effectivels skip ranges that are out of interest.
   "skip":              OptionInfo("list"), # "multiple: RE-character-set
   "skip_range":        OptionInfo("list"), # "multiple: RE-character-string RE-character-string
   "skip_nested_range": OptionInfo("list"), # "multiple: RE-character-string RE-character-string
}

#-----------------------------------------------------------------------------------------
# initial_mode: mode in which the lexcial analyser shall start
#-----------------------------------------------------------------------------------------
initial_mode = CodeFragment()

#-----------------------------------------------------------------------------------------
# header: code fragment that is to be pasted before mode transitions
#         and pattern action pairs (e.g. '#include<something>'
#-----------------------------------------------------------------------------------------
header = CodeFragment()

#-----------------------------------------------------------------------------------------
# class_body: code fragment that is to be pasted inside the class definition
#             of the lexical analyser class.
#-----------------------------------------------------------------------------------------
class_body = CodeFragment()

#-----------------------------------------------------------------------------------------
# class_init: code fragment that is to be pasted inside the lexer class constructor
#-----------------------------------------------------------------------------------------
class_init = CodeFragment()


class PatternShorthand:
    def __init__(self, Name="", StateMachine="", Filename="", LineN=-1, RE=""):
        assert StateMachine.has_origins() == False
        assert StateMachine.__class__.__name__ == "StateMachine"

        self.name               = Name
        self.state_machine      = StateMachine
        self.filename           = Filename
        self.line_n             = LineN
        self.regular_expression = RE

#-----------------------------------------------------------------------------------------
# shorthand_db: user defined names for regular expressions.
#               (this is what contained in the pattern file for flex-based engines.
#                it is only used with quex generated engines)   
#-----------------------------------------------------------------------------------------
shorthand_db = {}

#-----------------------------------------------------------------------------------------
# token_id_db: list of all defined token-ids together with the file position
#              where they are defined. See token_ide_maker, class TokenInfo.
#-----------------------------------------------------------------------------------------
token_id_db = {}


