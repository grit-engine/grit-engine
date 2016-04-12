import os
import sys
from   copy import deepcopy
sys.path.insert(0, os.environ["QUEX_PATH"])

from   quex.input.setup import setup as Setup
import quex.core_engine.state_machine.index as sm_index
import quex.core_engine.utf8 as utf8
from   quex.frs_py.string_handling               import blue_print
from   quex.core_engine.generator.drop_out       import get_forward_load_procedure
from   quex.core_engine.generator.languages.core import __nice
import quex.core_engine.generator.transition_block as transition_block
from   quex.core_engine.state_machine.transition_map import TransitionMap 



def do(SkipperDescriptor):
    LanguageDB = Setup.language_db
    skipper_class = SkipperDescriptor.__class__.__name__
    assert skipper_class in ["SkipperRange", "SkipperCharacterSet"]

    if skipper_class == "SkipperRange":
        return  create_skip_range_code(SkipperDescriptor.get_closing_sequence())
    elif skipper_class == "SkipperCharacterSet":
        return  create_skip_code(SkipperDescriptor.get_character_set())
    else:
        assert None

def create_skip_range_code(ClosingSequence):
    LanguageDB = Setup.language_db
    return  "{\n" \
            + LanguageDB["$comment"]("Range skipper state")                           \
            + get_range_skipper(ClosingSequence, LanguageDB) \
            + "\n}\n"

def create_skip_code(CharacterSet):
    LanguageDB = Setup.language_db
    return  "{\n" \
            + LanguageDB["$comment"]("Character set skipper state") \
            + get_character_set_skipper(CharacterSet, LanguageDB)   \
            + "\n}\n"

range_skipper_template = """
{
    $$DELIMITER_COMMENT$$
    const QUEX_CHARACTER_TYPE   Skipper$$SKIPPER_INDEX$$[] = { $$DELIMITER$$ };
    const size_t                Skipper$$SKIPPER_INDEX$$L  = $$DELIMITER_LENGTH$$;
    QUEX_CHARACTER_TYPE*        text_end = QuexBuffer_text_end(&me->buffer);
$$LC_COUNT_COLUMN_N_POINTER_DEFINITION$$

$$ENTRY$$
    QUEX_BUFFER_ASSERT_CONSISTENCY(&me->buffer);
    __quex_assert(QuexBuffer_content_size(&me->buffer) >= Skipper$$SKIPPER_INDEX$$L );

    /* NOTE: If _input_p == end of buffer, then it will drop out immediately out of the
     *       loop below and drop into the buffer reload procedure.                      */

    /* Loop eating characters: Break-out as soon as the First Character of the Delimiter
     * (FCD) is reached. Thus, the FCD plays also the role of the Buffer Limit Code. There
     * are two reasons for break-out:
     *    (1) we reached a limit (end-of-file or buffer-limit)
     *    (2) there was really the FCD in the character stream
     * This must be distinguished after the loop was exited. But, during the 'swallowing' we
     * are very fast, because we do not have to check for two different characters.        */
    *text_end = Skipper$$SKIPPER_INDEX$$[0]; /* Overwrite BufferLimitCode (BLC).  */
    $$WHILE_1_PLUS_1_EQUAL_2$$
        $$INPUT_GET$$ 
        $$IF_INPUT_EQUAL_DELIMITER_0$$
            $$BREAK$$
        $$ENDIF$$
$$LC_COUNT_IN_LOOP$$
        $$INPUT_P_INCREMENT$$ /* Now, BLC cannot occur. See above. */
    $$END_WHILE$$
    *text_end = QUEX_SETTING_BUFFER_LIMIT_CODE; /* Reset BLC. */

    /* Case (1) and (2) from above can be distinguished easily: 
     *
     *   (1) Distance to text end == 0: 
     *         End-of-File or Buffer-Limit. 
     *         => goto to drop-out handling
     *
     *   (2) Else:                      
     *         First character of delimit reached. 
     *         => For the verification of the tail of the delimiter it is 
     *            essential that it is loaded completely into the buffer. 
     *            For this, it must be required:
     *
     *                Distance to text end >= Delimiter length 
     *
     *                _input_p    end
     *                    |        |           end - _input_p >= 3
     *                [ ][R][E][M][#]          
     * 
     *         The case of reload should be seldom and is costy anyway. 
     *         Thus let's say, that in this case we simply enter the drop 
     *         out and start the search for the delimiter all over again.
     *
     *         (2.1) Distance to text end < Delimiter length
     *                => goto to drop-out handling
     *         (2.2) Start detection of tail of delimiter
     *
     */
    if( QuexBuffer_distance_input_to_text_end(&me->buffer) == 0 ) {
        /* (1) */
        $$GOTO_DROP_OUT$$            
    } 
    else if( Skipper$$SKIPPER_INDEX$$L && QuexBuffer_distance_input_to_text_end(&me->buffer) < Skipper$$SKIPPER_INDEX$$L ) {
        /* (2.1) */
        $$GOTO_DROP_OUT$$            
    }
$$LC_COUNT_AT_LOOP_EXIT$$
    /* (2.2) Test the remaining delimiter, but note, that the check must restart at '_input_p + 1'
     *       if any later check fails.                                                              */
    $$INPUT_P_INCREMENT$$
    /* Example: Delimiter = '*', '/'; if we get ...[*][*][/]... then the the first "*" causes 
     *          a drop out out of the 'swallowing loop' and the second "*" will mismatch 
     *          the required "/". But, then the second "*" must be presented to the
     *          swallowing loop and the letter after it completes the 'match'.
     * (The whole discussion, of course, is superflous if the range delimiter has length 1.)  */
$$DELIMITER_REMAINDER_TEST$$            
    {
        $$SET_INPUT_P_BEHIND_DELIMITER$$ 
        /* NOTE: The initial state does not increment the input_p. When it detects that
         * it is located on a buffer border, it automatically triggers a reload. No 
         * need here to reload the buffer. */
$$LC_COUNT_END_PROCEDURE$$
        $$GOTO_REENTRY_PREPARATION$$ /* End of range reached. */
    }

$$DROP_OUT$$
    QUEX_BUFFER_ASSERT_CONSISTENCY_LIGHT(&me->buffer);
    /* -- When loading new content it is checked that the beginning of the lexeme
     *    is not 'shifted' out of the buffer. In the case of skipping, we do not care about
     *    the lexeme at all, so do not restrict the load procedure and set the lexeme start
     *    to the actual input position.                                                    */
    /* -- According to case (2.1) is is possible that the _input_p does not point to the end
     *    of the buffer, thus we record the current position in the lexeme start pointer and
     *    recover it after the loading. */
    $$MARK_LEXEME_START$$
    me->buffer._input_p = text_end;
$$LC_COUNT_BEFORE_RELOAD$$
    if(    QuexAnalyser_buffer_reload_forward(&me->buffer, &last_acceptance_input_position,
                                              post_context_start_position, PostContextStartPositionN) ) {
        /* Recover '_input_p' from lexeme start 
         * (inverse of what we just did before the loading) */
        me->buffer._input_p = me->buffer._lexeme_start_p;
        /* After reload, we need to increment _input_p. That's how the game is supposed to be played. 
         * But, we recovered from lexeme start pointer, and this one does not need to be incremented. */
        text_end = QuexBuffer_text_end(&me->buffer);
$$LC_COUNT_AFTER_RELOAD$$
        QUEX_BUFFER_ASSERT_CONSISTENCY(&me->buffer);
        $$GOTO_ENTRY$$
    }
    /* Here, either the loading failed or it is not enough space to carry a closing delimiter */
    me->buffer._input_p = me->buffer._lexeme_start_p;
    $$MISSING_CLOSING_DELIMITER$$
}
"""

def get_range_skipper(EndSequence, LanguageDB, MissingClosingDelimiterAction=""):
    assert EndSequence.__class__  == list
    assert len(EndSequence) >= 1
    assert map(type, EndSequence) == [int] * len(EndSequence)

    # Name the $$SKIPPER$$
    skipper_index = sm_index.get()

    # Determine the $$DELIMITER$$
    delimiter_str = ""
    delimiter_comment_str = "                         Delimiter: "
    for letter in EndSequence:
        delimiter_comment_str += "'%s', " % utf8.map_unicode_to_utf8(letter)
        delimiter_str += "0x%X, " % letter
    delimiter_length_str = "%i" % len(EndSequence)
    delimiter_comment_str = LanguageDB["$comment"](delimiter_comment_str) 

    # Determine the check for the tail of the delimiter
    delimiter_remainder_test_str = ""
    if len(EndSequence) != 1: 
        txt = ""
        i = 0
        for letter in EndSequence[1:]:
            i += 1
            txt += "    " + LanguageDB["$input/get-offset"](i-1) + "\n"
            txt += "    " + LanguageDB["$if !="]("Skipper$$SKIPPER_INDEX$$[%i]" % i)
            txt += "         " + LanguageDB["$goto"]("$entry", skipper_index) + "\n"
            txt += "    " + LanguageDB["$endif"]
        delimiter_remainder_test_str = txt

    # The main part
    code_str = blue_print(range_skipper_template,
                          [["$$DELIMITER$$",                  delimiter_str],
                           ["$$DELIMITER_LENGTH$$",           delimiter_length_str],
                           ["$$DELIMITER_COMMENT$$",          delimiter_comment_str],
                           ["$$WHILE_1_PLUS_1_EQUAL_2$$",     LanguageDB["$loop-start-endless"]],
                           ["$$END_WHILE$$",                  LanguageDB["$loop-end"]],
                           ["$$INPUT_P_INCREMENT$$",          LanguageDB["$input/increment"]],
                           ["$$INPUT_P_DECREMENT$$",          LanguageDB["$input/decrement"]],
                           ["$$INPUT_GET$$",                  LanguageDB["$input/get"]],
                           ["$$IF_INPUT_EQUAL_DELIMITER_0$$", LanguageDB["$if =="]("Skipper$$SKIPPER_INDEX$$[0]")],
                           ["$$BREAK$$",                      LanguageDB["$break"]],
                           ["$$ENDIF$$",                      LanguageDB["$endif"]],
                           ["$$ENTRY$$",                      LanguageDB["$label-def"]("$entry", skipper_index)],
                           ["$$DROP_OUT$$",                   LanguageDB["$label-def"]("$drop-out", skipper_index)],
                           ["$$GOTO_ENTRY$$",                 LanguageDB["$goto"]("$entry", skipper_index)],
                           ["$$GOTO_REENTRY_PREPARATION$$",   LanguageDB["$goto"]("$re-start")],
                           ["$$MARK_LEXEME_START$$",          LanguageDB["$mark-lexeme-start"]],
                           ["$$DELIMITER_REMAINDER_TEST$$",   delimiter_remainder_test_str],
                           ["$$SET_INPUT_P_BEHIND_DELIMITER$$", LanguageDB["$input/add"](len(EndSequence)-1)],
                           ["$$MISSING_CLOSING_DELIMITER$$",  MissingClosingDelimiterAction],
                          ])

    # Line and column number counting
    code_str = __range_skipper_lc_counting_replacements(code_str, EndSequence)

    # The finishing touch
    code_str = blue_print(code_str,
                          [["$$SKIPPER_INDEX$$", __nice(skipper_index)],
                           ["$$GOTO_DROP_OUT$$", LanguageDB["$goto"]("$drop-out", skipper_index)]])

    return code_str


trigger_set_skipper_template = """
{ 
    $$DELIMITER_COMMENT$$
$$LC_COUNT_COLUMN_N_POINTER_DEFINITION$$

    QUEX_BUFFER_ASSERT_CONSISTENCY(&me->buffer);
    __quex_assert(QuexBuffer_content_size(&me->buffer) >= 1);
    if( QuexBuffer_distance_input_to_text_end(&me->buffer) == 0 ) 
        $$GOTO_DROP_OUT$$

    /* NOTE: For simple skippers the end of content does not have to be overwriten 
     *       with anything (as done for range skippers). This is so, because the abort
     *       criteria is that a character occurs which does not belong to the trigger 
     *       set. The BufferLimitCode, though, does never belong to any trigger set and
     *       thus, no special character is to be set.                                   */
$$LOOP_START$$
    $$INPUT_GET$$ 
$$LC_COUNT_IN_LOOP$$
$$ON_TRIGGER_SET_TO_LOOP_START$$
$$LOOP_REENTRANCE$$
    $$INPUT_P_INCREMENT$$ /* Now, BLC cannot occur. See above. */
    $$GOTO_LOOP_START$$

$$DROP_OUT$$
    /* -- When loading new content it is always taken care that the beginning of the lexeme
     *    is not 'shifted' out of the buffer. In the case of skipping, we do not care about
     *    the lexeme at all, so do not restrict the load procedure and set the lexeme start
     *    to the actual input position.                                                   
     * -- The input_p will at this point in time always point to the buffer border.        */
    if( QuexBuffer_distance_input_to_text_end(&me->buffer) == 0 ) {
        QUEX_BUFFER_ASSERT_CONSISTENCY(&me->buffer);
$$LC_COUNT_BEFORE_RELOAD$$
        $$MARK_LEXEME_START$$
        if( QuexAnalyser_buffer_reload_forward(&me->buffer, &last_acceptance_input_position,
                                               post_context_start_position, PostContextStartPositionN) ) {

            QUEX_BUFFER_ASSERT_CONSISTENCY(&me->buffer);
            QuexBuffer_input_p_increment(&me->buffer);
$$LC_COUNT_AFTER_RELOAD$$
            $$GOTO_LOOP_START$$
        } else {
            $$GOTO_TERMINAL_EOF$$
        }
    }

$$DROP_OUT_DIRECT$$
$$LC_COUNT_END_PROCEDURE$$
    /* There was no buffer limit code, so no end of buffer or end of file --> continue analysis 
     * The character we just swallowed must be re-considered by the main state machine.
     * But, note that the initial state does not increment '_input_p'!
     */
    $$GOTO_REENTRY_PREPARATION$$                           
}
"""

def get_character_set_skipper(TriggerSet, LanguageDB):
    """This function implements simple 'skipping' in the sense of passing by
       characters that belong to a given set of characters--the TriggerSet.
    """
    assert TriggerSet.__class__.__name__ == "NumberSet"
    assert not TriggerSet.is_empty()

    skipper_index = sm_index.get()
    # Mini trigger map:  [ trigger set ] --> loop start
    # That means: As long as characters of the trigger set appear, we go to the loop start.
    transition_map = TransitionMap()
    transition_map.add_transition(TriggerSet, skipper_index)
    iteration_code = transition_block.do(transition_map.get_trigger_map(), skipper_index, InitStateF=False, DSM=None)

    comment_str = LanguageDB["$comment"]("Skip any character in " + TriggerSet.get_utf8_string())

    # Line and column number counting
    code_str = __set_skipper_lc_counting_replacements(trigger_set_skipper_template, TriggerSet)

    # The finishing touch
    txt = blue_print(code_str,
                      [
                       ["$$DELIMITER_COMMENT$$",          comment_str],
                       ["$$INPUT_P_INCREMENT$$",          LanguageDB["$input/increment"]],
                       ["$$INPUT_P_DECREMENT$$",          LanguageDB["$input/decrement"]],
                       ["$$INPUT_GET$$",                  LanguageDB["$input/get"]],
                       ["$$IF_INPUT_EQUAL_DELIMITER_0$$", LanguageDB["$if =="]("SkipDelimiter$$SKIPPER_INDEX$$[0]")],
                       ["$$ENDIF$$",                      LanguageDB["$endif"]],
                       ["$$LOOP_START$$",                 LanguageDB["$label-def"]("$input", skipper_index)],
                       ["$$GOTO_LOOP_START$$",            LanguageDB["$goto"]("$input", skipper_index)],
                       ["$$LOOP_REENTRANCE$$",            LanguageDB["$label-def"]("$entry", skipper_index)],
                       ["$$RESTART$$",                    LanguageDB["$label-def"]("$input", skipper_index)],
                       ["$$DROP_OUT$$",                   LanguageDB["$label-def"]("$drop-out", skipper_index)],
                       ["$$DROP_OUT_DIRECT$$",            LanguageDB["$label-def"]("$drop-out-direct", skipper_index)],
                       ["$$GOTO_LOOP_START$$",            LanguageDB["$goto"]("$entry", skipper_index)],
                       ["$$SKIPPER_INDEX$$",              repr(skipper_index)],
                       ["$$GOTO_TERMINAL_EOF$$",          LanguageDB["$goto"]("$terminal-EOF")],
                       ["$$GOTO_REENTRY_PREPARATION$$",   LanguageDB["$goto"]("$re-start")],
                       ["$$MARK_LEXEME_START$$",          LanguageDB["$mark-lexeme-start"]],
                       ["$$ON_TRIGGER_SET_TO_LOOP_START$$", iteration_code],
                      ])

    return blue_print(txt,
                       [["$$GOTO_DROP_OUT$$", LanguageDB["$goto"]("$drop-out", skipper_index)]])

def get_nested_character_skipper(StartSequence, EndSequence, LanguageDB, BufferEndLimitCode):
    assert StartSequence.__class__  == list
    assert len(StartSequence)       >= 1
    assert map(type, StartSequence) == [int] * len(StartSequence)
    assert EndSequence.__class__  == list
    assert len(EndSequence)       >= 1
    assert map(type, EndSequence) == [int] * len(EndSequence)
    assert StartSequence != EndSequence

    # Identify the common start of 'StartSequence' and 'EndSequence'
    CommonSequence    = []
    StartSequenceTail = []  # 'un-common' tail of the start sequence
    EndSequenceTail   = []  # 'un-common' tail of the end sequence
    L_min             = min(len(StartSequence), len(EndSequence))
    characters_from_begin_to_i_are_common_f = True
    for i in range(L_min):
        if (not characters_from_begin_to_i_are_common_f) or (StartSequence[i] != EndSequence[i]): 
            StartSequenceTail.append(StartSequence[i])
            EndSequenceTail.append(EndSequence[i])
            characters_from_begin_to_i_are_common_f = False
        else: 
            CommonSequence.append(StartSequence[i])

    if CommonSequence != []:
        msg += "        " + LanguageDB["$if =="](repr(CommonSequence[0]))
        msg += "            " + action_on_first_character_match
        msg += "        " + LanguageDB["$endif"]
    else:
        msg += "        " + LanguageDB["$if =="](repr(StartSequenceTail[0]))
        msg += "            " + action_on_first_character_match
        msg += "        " + LanguageDB["$endif"]


lc_counter_in_loop = """
#       if defined(QUEX_OPTION_LINE_NUMBER_COUNTING) || defined(QUEX_OPTION_COLUMN_NUMBER_COUNTING)
        if( input == (QUEX_CHARACTER_TYPE)'\\n' ) { 
            ++(self.counter._line_number_at_end);
#           if defined(QUEX_OPTION_COLUMN_NUMBER_COUNTING)
            column_count_p_$$SKIPPER_INDEX$$ = QuexBuffer_tell_memory_adr(&me->buffer);
            self.counter._column_number_at_end = 0;
#           endif
        }
#       endif
"""
def __range_skipper_lc_counting_replacements(code_str, EndSequence):
    """Line and Column Number Counting(Range Skipper):
     
         -- in loop if there appears a newline, then do:
            increment line_n
            set position from where to count column_n
         -- at end of skipping do one of the following:
            if end delimiter contains newline:
               column_n = number of letters since last new line in end delimiter
               increment line_n by number of newlines in end delimiter.
               (NOTE: in this case the setting of the position from where to count
                      the column_n can be omitted.)
            else:
               column_n = current_position - position from where to count column number.

       NOTE: On reload we do count the column numbers and reset the column_p.
    """
    variable_definition = \
      "#   if defined(QUEX_OPTION_LINE_NUMBER_COUNTING) || defined(QUEX_OPTION_COLUMN_NUMBER_COUNTING)\n" + \
      "#   ifdef QUEX_OPTION_COLUMN_NUMBER_COUNTING\n" + \
      "    QUEX_CHARACTER_POSITION_TYPE column_count_p_$$SKIPPER_INDEX$$ = QuexBuffer_tell_memory_adr(&me->buffer);\n"+\
      "#   endif\n" + \
      "#   endif\n"
    in_loop       = ""
    end_procedure = ""
    exit_loop     = ""
    new_line_detection_in_loop_enabled_f = True

    # Does the end delimiter contain a newline?
    try:    index = EndSequence.index(ord("\n"))
    except: index = -1
    if index != -1:
        if index == 0:
            # Inside the skipped range, there cannot have been a newline
            new_line_detection_in_loop_enabled_f = False
            exit_loop = "#       ifdef QUEX_OPTION_LINE_NUMBER_COUNTING\n" + \
                        "        ++(self.counter._line_number_at_end); /* First limit character was the newline */\n" \
                        "#       endif" 

        # If the first character in the delimiter is newline, then it was counted alread, see above.
        delimiter_newline_n = EndSequence[1:].count(ord("\n"))
        if delimiter_newline_n != 0:
            end_procedure += "#       ifdef QUEX_OPTION_LINE_NUMBER_COUNTING\n" + \
                             "        self.counter._line_number_at_end += %i;\n" % delimiter_newline_n + \
                             "#       endif\n"

        # If delimiter contains newline, then the column number is identical to the distance
        # of the last newline to the end of the delimiter.
        dummy = deepcopy(EndSequence)
        dummy.reverse()
        delimiter_tail_n    = dummy.index(ord("\n")) + 1
        if delimiter_tail_n != 0:
            end_procedure += "#       ifdef QUEX_OPTION_COLUMN_NUMBER_COUNTING\n" + \
                             "        self.counter._column_number_at_end = %i;\n" % delimiter_tail_n + \
                             "#       endif\n"
    else:
        end_procedure = "#       ifdef QUEX_OPTION_COLUMN_NUMBER_COUNTING\n" + \
                        "        self.counter._column_number_at_end +=   QuexBuffer_tell_memory_adr(&me->buffer)\n" + \
                        "                                              - column_count_p_$$SKIPPER_INDEX$$;\n" + \
                        "#   endif\n"
    before_reload  = "#   ifdef QUEX_OPTION_COLUMN_NUMBER_COUNTING\n" + \
                     "    self.counter._column_number_at_end +=  QuexBuffer_tell_memory_adr(&me->buffer)\n" + \
                     "                                         - column_count_p_$$SKIPPER_INDEX$$;\n" + \
                     "#   endif\n"
    after_reload   = "#       ifdef QUEX_OPTION_COLUMN_NUMBER_COUNTING\n" + \
                     "        column_count_p_$$SKIPPER_INDEX$$ = QuexBuffer_tell_memory_adr(&me->buffer);\n" + \
                     "#       endif\n"

    if new_line_detection_in_loop_enabled_f:
        in_loop = lc_counter_in_loop

    return blue_print(code_str,
                     [["$$LC_COUNT_COLUMN_N_POINTER_DEFINITION$$", variable_definition],
                      ["$$LC_COUNT_IN_LOOP$$",                     in_loop],
                      ["$$LC_COUNT_END_PROCEDURE$$",               end_procedure],
                      ["$$LC_COUNT_BEFORE_RELOAD$$",               before_reload],
                      ["$$LC_COUNT_AFTER_RELOAD$$",                after_reload],
                      ["$$LC_COUNT_AT_LOOP_EXIT$$",                exit_loop],
                      ])


def __set_skipper_lc_counting_replacements(code_str, CharacterSet):
    """Line and Column Number Counting(Range Skipper):
     
         -- in loop if there appears a newline, then do:
            increment line_n
            set position from where to count column_n
         -- at end of skipping do one of the following:
            if end delimiter contains newline:
               column_n = number of letters since last new line in end delimiter
               increment line_n by number of newlines in end delimiter.
               (NOTE: in this case the setting of the position from where to count
                      the column_n can be omitted.)
            else:
               column_n = current_position - position from where to count column number.

       NOTE: On reload we do count the column numbers and reset the column_p.
    """
    variable_definition = \
      "#   if defined(QUEX_OPTION_LINE_NUMBER_COUNTING) || defined(QUEX_OPTION_COLUMN_NUMBER_COUNTING)\n" + \
      "#   ifdef QUEX_OPTION_COLUMN_NUMBER_COUNTING\n" + \
      "    QUEX_CHARACTER_POSITION_TYPE column_count_p_$$SKIPPER_INDEX$$ = QuexBuffer_tell_memory_adr(&me->buffer);\n"+\
      "#   endif\n" + \
      "#   endif\n"
    in_loop       = ""
    end_procedure = ""
    exit_loop     = ""

    # Does the end delimiter contain a newline?
    if CharacterSet.contains(ord("\n")):
        in_loop = lc_counter_in_loop

    end_procedure = "#       ifdef QUEX_OPTION_COLUMN_NUMBER_COUNTING\n" + \
                    "        self.counter._column_number_at_end +=   QuexBuffer_tell_memory_adr(&me->buffer)\n" + \
                    "                                              - column_count_p_$$SKIPPER_INDEX$$;\n" + \
                    "#       endif\n"
    before_reload  = "#      ifdef QUEX_OPTION_COLUMN_NUMBER_COUNTING\n" + \
                     "       self.counter._column_number_at_end +=  QuexBuffer_tell_memory_adr(&me->buffer)\n" + \
                     "                                            - column_count_p_$$SKIPPER_INDEX$$;\n" + \
                     "#      endif\n"
    after_reload   = "#          ifdef QUEX_OPTION_COLUMN_NUMBER_COUNTING\n" + \
                     "           column_count_p_$$SKIPPER_INDEX$$ = QuexBuffer_tell_memory_adr(&me->buffer);\n" + \
                     "#          endif\n"

    return blue_print(code_str,
                     [["$$LC_COUNT_COLUMN_N_POINTER_DEFINITION$$", variable_definition],
                      ["$$LC_COUNT_IN_LOOP$$",                     in_loop],
                      ["$$LC_COUNT_END_PROCEDURE$$",               end_procedure],
                      ["$$LC_COUNT_BEFORE_RELOAD$$",               before_reload],
                      ["$$LC_COUNT_AFTER_RELOAD$$",                after_reload],
                      ])
