from StringIO import StringIO
from   quex.input.setup import setup as Setup

from   quex.frs_py.file_in          import EndOfStreamException, error_msg
from   quex.exception               import RegularExpressionException
import quex.lexer_mode              as lexer_mode
import quex.core_engine.regular_expression.core as regex
import quex.core_engine.regular_expression.character_set_expression as charset_expression
import quex.core_engine.regular_expression.snap_character_string as snap_character_string

def parse(fh, AllowNothingIsFineF=False):

    start_position = fh.tell()
    try:
        # (*) parse regular expression, build state machine
        pattern_state_machine = regex.do(fh, lexer_mode.shorthand_db, 
                                         BufferLimitCode            = Setup.buffer_limit_code,
                                         DOS_CarriageReturnNewlineF = not Setup.no_dos_carriage_return_newline_f,
                                         AllowNothingIsFineF        = AllowNothingIsFineF)

        # (*) error in regular expression?
        if pattern_state_machine == None:
            error_msg("No valid regular expression detected.", fh)

    except RegularExpressionException, x:
        fh.seek(start_position)
        error_msg("Regular expression parsing:\n" + x.message, fh)

    except EndOfStreamException:
        fh.seek(start_position)
        error_msg("End of file reached while parsing regular expression.", fh)

    end_position = fh.tell()

    fh.seek(start_position)
    regular_expression = fh.read(end_position - start_position)

    return regular_expression, pattern_state_machine


def parse_character_set(Txt_or_File, PatternStringF=False):

    if Txt_or_File.__class__ in [file, StringIO]:
        sh       = Txt_or_File
        sh_ref   = sh
        position = sh.tell()
    else:
        sh     = StringIO(Txt_or_File)
        sh_ref = -1

    start_position = sh.tell()

    try:
        # -- parse regular expression, build state machine
        character_set = charset_expression.snap_set_expression(sh)

        if character_set == None:
            error_msg("No valid regular character set expression detected.", sh_ref)

        # -- character set is not supposed to contain buffer limit code
        if character_set.contains(Setup.buffer_limit_code):
            character_set.cut_interval(Interval(Setup.buffer_limit_code, Setup.buffer_limit_code))

    except RegularExpressionException, x:
        error_msg("Regular expression parsing:\n" + x.message, sh_ref)

    except EndOfStreamException:
        if sh_ref != -1: sh_ref.seek(position)
        error_msg("End of character set expression reached while parsing.", sh_ref)

    if PatternStringF:
        assert sh.__class__ != StringIO
        end_position = sh.tell()
        sh.seek(start_position)
        regular_expression = sh.read(end_position - start_position)
        return regular_expression, character_set

    return character_set

def parse_character_string(Txt_or_File, PatternStringF=False):

    if Txt_or_File.__class__ in [file, StringIO]:
        sh       = Txt_or_File
        sh_ref   = sh
        position = sh.tell()
    else:
        sh     = StringIO(Txt_or_File)
        sh_ref = -1

    start_position = sh.tell()

    try:
        # -- parse regular expression, build state machine
        state_machine = snap_character_string.do(sh)
        state_machine = regex.__clean_and_validate(state_machine, 
                                                   Setup.buffer_limit_code, 
                                                   AllowNothingIsFineF=False)

        if state_machine == None:
            error_msg("No valid regular character string expression detected.", sh_ref)

    except RegularExpressionException, x:
        error_msg("Regular expression parsing:\n" + x.message, sh_ref)

    except EndOfStreamException:
        if sh_ref != -1: sh_ref.seek(position)
        error_msg("End of character string reached while parsing.", sh_ref)

    if PatternStringF:
        assert sh.__class__ != StringIO
        end_position = sh.tell()
        sh.seek(start_position)
        regular_expression = sh.read(end_position - start_position)
        return regular_expression, state_machine

    return state_machine

