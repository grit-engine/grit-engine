from copy import copy
import os
import sys

from   quex.frs_py.file_in import error_msg

from   quex.input.setup import setup as Setup
import quex.token_id_maker                      as token_id_maker
import quex.lexer_mode                          as lexer_mode
from   quex.core_engine.generator.action_info   import UserCodeFragment_straighten_open_line_pragmas, \
                                                       CodeFragment
from quex.token_id_maker                        import TokenInfo

import quex.core_engine.generator.core          as generator
from   quex.core_engine.generator.action_info   import PatternActionInfo
import quex.input.quex_file_parser              as quex_file_parser
import quex.consistency_check                   as consistency_check
import quex.output.cpp.core                     as quex_class_out
import quex.output.cpp.action_code_formatter    as action_code_formatter
import quex.output.graphviz.interface           as plot_generator

def do():
    """Generates state machines for all modes. Each mode results into 
       a separate state machine that is stuck into a virtual function
       of a class derived from class 'quex_mode'.
    """
    # NOTE: The generated header file that contains the lexical analyser class includes
    #       the file "code_base/code_base/definitions-quex-buffer.h". But the analyser
    #       functions also need 'connection' to the lexer class, so we include the header
    #       of the generated lexical analyser at this place.
    lexer_mode.token_id_db["TERMINATION"]   = TokenInfo("TERMINATION",   ID=Setup.token_id_termination)
    lexer_mode.token_id_db["UNINITIALIZED"] = TokenInfo("UNINITIALIZED", ID=Setup.token_id_uninitialized)

    mode_db = __get_mode_db(Setup)

    # (*) get list of modes that are actually implemented
    #     (abstract modes only serve as common base)
    mode_list      = filter(lambda mode: mode.options["inheritable"] != "only", mode_db.values())
    mode_name_list = map(lambda mode: mode.name, mode_list)

    # (2) implement the 'quex' core class from a template
    #
    # -- do the coding of the class framework
    quex_class_out.do(mode_db, Setup)

    # (3) create the token ids
    token_id_maker.do(Setup) 

    # (3) implement the lexer mode-specific analyser functions
    inheritance_info_str  = "Information about what pattern 'comes' from what mode in the inheritance tree.\n\n"
    inheritance_info_str += "[1] pattern, [2] dominating mode, [3] dominating inheritance level, [4] pattern index\n\n"
    analyzer_code = ""
    for mode in mode_list:        
        # accumulate inheritance information for comment
        x, y = get_code_for_mode(mode, mode_name_list) 
        analyzer_code        += x
        inheritance_info_str += "(%s)\n" % mode.name + y + "\n\n"
        
    # find unused labels
    analyzer_code = generator.delete_unused_labels(analyzer_code)

    # generate frame for analyser code
    analyzer_code = generator.frame_this(analyzer_code)

    # Bring the info about the patterns first
    inheritance_info_str = inheritance_info_str.replace("*/", "* /")
    inheritance_info_str = inheritance_info_str.replace("/*", "/ *")
    analyzer_code = Setup.language_db["$ml-comment"](inheritance_info_str) + "\n" + analyzer_code

    # write code to a header file
    fh = open(Setup.output_core_engine_file, "wb")
    if os.linesep != "\n": analyzer_code = analyzer_code.replace("\n", os.linesep)
    fh.write(analyzer_code)
    fh.close()

    UserCodeFragment_straighten_open_line_pragmas(Setup.output_file_stem, "C")
    UserCodeFragment_straighten_open_line_pragmas(Setup.output_core_engine_file, "C")
    UserCodeFragment_straighten_open_line_pragmas(Setup.output_code_file, "C")

def get_code_for_mode(Mode, ModeNameList):

    # -- some modes only define event handlers that are inherited
    if not Mode.has_matches(): return "", ""

    # -- 'end of stream' action
    if Mode.on_end_of_stream_code_fragments() == []:
        txt  = "self.send(%sTERMINATION);\n" % Setup.input_token_id_prefix 
        txt += "#ifdef __QUEX_OPTION_ANALYSER_RETURN_TYPE_IS_VOID\n"
        txt += "    return /*__QUEX_TOKEN_ID_TERMINATION*/;\n"
        txt += "#else\n"
        txt += "    return __QUEX_TOKEN_ID_TERMINATION;\n"
        txt += "#endif\n"
        Mode.on_end_of_stream = CodeFragment(txt)

    end_of_stream_action = action_code_formatter.do(Mode, Mode.on_end_of_stream_code_fragments(), 
                                                    "on_end_of_stream", None, EOF_ActionF=True)
    # -- 'default' action (nothing matched)

    if Mode.on_failure_code_fragments() == []:
        txt  = "self.send(%sTERMINATION);\n" % Setup.input_token_id_prefix 
        txt += "#ifdef __QUEX_OPTION_ANALYSER_RETURN_TYPE_IS_VOID\n"
        txt += "    return /*__QUEX_TOKEN_ID_TERMINATION*/;\n"
        txt += "#else\n"
        txt += "    return __QUEX_TOKEN_ID_TERMINATION;\n"
        txt += "#endif\n"
        Mode.on_failure = CodeFragment(txt)

    default_action = action_code_formatter.do(Mode, Mode.on_failure_code_fragments(), 
                                              "on_failure", None, Default_ActionF=True)

    # -- adapt pattern-action pair information so that it can be treated
    #    by the code generator.
    inheritance_info_str, pattern_action_pair_list = get_generator_input(Mode)

    analyzer_code = generator.do(pattern_action_pair_list, 
                                 DefaultAction                  = PatternActionInfo(None, default_action), 
                                 EndOfStreamAction              = PatternActionInfo(None, end_of_stream_action),
                                 PrintStateMachineF             = True,
                                 StateMachineName               = Mode.name,
                                 AnalyserStateClassName         = Setup.output_engine_name,
                                 StandAloneAnalyserF            = False, 
                                 QuexEngineHeaderDefinitionFile = Setup.output_file_stem,
                                 ModeNameList                   = ModeNameList)

    return analyzer_code, inheritance_info_str
    
def get_generator_input(Mode):
    """The module 'quex.core_engine.generator.core' produces the code for the 
       state machine. However, it requires a certain data format. This function
       adapts the mode information to this format. Additional code is added 

       -- for counting newlines and column numbers. This happens inside
          the function ACTION_ENTRY().
       -- (optional) for a virtual function call 'on_action_entry()'.
       -- (optional) for debug output that tells the line number and column number.
    """
    match_info_list = Mode.pattern_action_pairs().values()

    # (*) sort the patterns:
    #
    #     -- The order of the patterns determine the sequence of the creation of the 
    #        state machines for the patterns.
    #     -- The sequence of the state machines determines the state machine ids. Patterns
    #        that are created earlier have a 'smaller' state machine id than patterns 
    #        that are created later. 
    #     -- The state machine id stands for the 'privilidge' of a pattern. A pattern
    #        with a lower state machine id has a higher priviledge than a pattern with
    #        a higher id. The state machine id is used for **sorting** during code
    #        generation.
    #
    #   A mode potentially accumulates inherited patterns from base modes.
    #   => At this place all patterns are sorted according to their inheritance 
    #      level. Thus, a propper priviledge handling is guaranteed.
    def pattern_precedence(A, B):
        tmp = - cmp(A.inheritance_level, B.inheritance_level)
        if tmp != 0: return tmp
        else:        return cmp(A.pattern_index(), B.pattern_index())
        
    match_info_list.sort(pattern_precedence)

    inheritance_info_str     = ""
    pattern_action_pair_list = []
    for pattern_info in match_info_list:
        safe_pattern_str      = pattern_info.pattern.replace("\"", "\\\"")
        pattern_state_machine = pattern_info.pattern_state_machine()

        # Prepare the action code for the analyzer engine. For this purpose several things
        # are be added to the user's code.
        prepared_action = action_code_formatter.do(Mode, pattern_info.action(), safe_pattern_str,
                                                   pattern_state_machine)

        action_info = PatternActionInfo(pattern_state_machine, prepared_action)

        pattern_action_pair_list.append(action_info)

        inheritance_info_str += " %s %s %2i %05i\n" % (safe_pattern_str,
                                                       pattern_info.inheritance_mode_name,
                                                       pattern_info.inheritance_level, 
                                                       pattern_info.pattern_index())
    
    return inheritance_info_str, pattern_action_pair_list

def __get_post_context_n(match_info_list):
    n = 0
    for info in MatchInfoList:
        if info.pattern_state_machine().core().post_context_id() != -1L:
            n += 1
    return n

def do_plot():

    mode_db = __get_mode_db(Setup)

    for mode in mode_db.values():        
        # -- some modes only define event handlers that are inherited
        if not mode.has_matches(): continue

        # -- adapt pattern-action pair information so that it can be treated
        #    by the code generator.
        dummy, pattern_action_pair_list = get_generator_input(mode)

        plotter = plot_generator.Generator(pattern_action_pair_list, 
                                           StateMachineName = mode.name,
                                           GraphicFormat    = Setup.plot_graphic_format)
        plotter.do()

def __get_mode_db(Setup):
    # (0) check basic assumptions
    if Setup.input_mode_files == []: error_msg("No input files.")
    
    # (1) input: do the pattern analysis, in case exact counting of newlines is required
    #            (this might speed up the lexer, but nobody might care ...)
    #            pattern_db = analyse_patterns.do(pattern_file)    
    mode_db = quex_file_parser.do(Setup.input_mode_files, Setup)

    # (*) perform consistency check 
    consistency_check.do(mode_db)

    return mode_db

