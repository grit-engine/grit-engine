import sys
import StringIO
import quex.core_engine.utf8 as utf8
import quex.core_engine.regular_expression.snap_backslashed_character as snap_backslashed_character
from quex.core_engine.interval_handling  import *
from quex.exception                      import RegularExpressionException



class Tracker:
    def __init__(self):
        self.match_set  = NumberSet()
        self.negation_f = False
 
    def consider_interval(self, Begin, End):
        if Begin > End:
            raise RegularExpressionException("Character range: '-' requires character with 'lower code' to preceed\n" + \
                                             "found range '%s-%s' which corresponds to %i-%i as unicode code points." % \
                                             (utf8.map_unicode_to_utf8(Begin), utf8.map_unicode_to_utf8(End), Begin, End))

        self.match_set.add_interval(Interval(Begin, End))

    def consider_letter(self, CharCode):
        self.consider_interval(CharCode, CharCode+1)
 
def do_string(UTF8_String):
    return do(StringIO.StringIO(UTF8_String))

def do(sh):
    """Transforms an expression of the form [a-z0-9A-Z] into a NumberSet of
       code points that corresponds to the characters and character ranges mentioned.
    """
    assert     sh.__class__.__name__ == "StringIO" \
            or sh.__class__.__name__ == "file"

    def __check_letter(stream, letter):
        position = stream.tell()
        if stream.read(1) == letter: return True
        else:                        stream.seek(position); return False

    # check, if the set is thought to be inverse (preceeded by '^')
    tracker = Tracker()

    if __check_letter(sh, "^"): tracker.negation_f = True

    char_code = None
    while char_code != 0xFF:
        char_code = utf8.__read_one_utf8_code_from_stream(sh)

        if char_code == ord("-"):
            raise RegularExpressionException("Character range operator '-' requires a preceding character as in 'a-z'.")
        elif char_code == 0xFF: 
            raise RegularExpressionException("Missing closing ']' in character range expression.")
        elif char_code == ord("]"):
            break
        elif char_code == ord("\\"):
            char_code = snap_backslashed_character.do(sh)

        if not __check_letter(sh, "-"): 
            # (*) Normal character
            tracker.consider_letter(char_code)
        else:
            # (*) Character range:  'character0' '-' 'character1'
            char_code_2 = utf8.__read_one_utf8_code_from_stream(sh)
            if char_code_2 in [0xFF, ord(']')]: 
                raise RegularExpressionException("Character range: '-' requires a character following '-'.")
            elif char_code == ord("-"):
                raise RegularExpressionException("Character range operator '-' followed by '-'.")
            elif char_code_2 == ord("\\"): 
                char_code_2 = snap_backslashed_character.do(sh)  

            # value denotes 'end', i.e first character outside the interval => add 1
            tracker.consider_interval(char_code, char_code_2 + 1)

    if tracker.negation_f: return tracker.match_set.inverse()
    else:                  return tracker.match_set


