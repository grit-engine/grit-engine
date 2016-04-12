import quex.core_engine.utf8                                          as utf8
import quex.core_engine.regular_expression.snap_backslashed_character as snap_backslashed_character
from quex.core_engine.state_machine.core import StateMachine
from quex.exception                      import  RegularExpressionException


def do(sh):
    """Converts a uni-code string into a state machine that parses 
       its letters sequentially. Each state in the sequence correponds
       to the sucessful triggering of a letter. Only the last state, though,
       is an acceptance state. Any bailing out before is 'not accepted'. 
       Example:

       "hey" is translated into the state machine:

           (0)-- 'h' -->(1)-- 'e' -->(2)-- 'y' --> ACCEPTANCE
            |            |            |
           FAIL         FAIL         FAIL
    
      Note: The state indices are globally unique. But, they are not necessarily
            0, 1, 2, ... 
    """
    assert     sh.__class__.__name__ == "StringIO" \
            or sh.__class__.__name__ == "file"

    # resulting state machine
    result    = StateMachine()
    state_idx = result.init_state_index

    # Only \" is a special character '"', any other backslashed character
    # remains as the sequence 'backslash' + character
    for char_code in get_character_code_sequence(sh):
        state_idx = result.add_transition(state_idx, char_code)

    # when the last state has trigger it is supposed to end up in 'acceptance'
    result.states[state_idx].set_acceptance()
    return result

def get_character_code_sequence(sh):
    assert     sh.__class__.__name__ == "StringIO" \
            or sh.__class__.__name__ == "file"

    # Only \" is a special character '"', any other backslashed character
    # remains as the sequence 'backslash' + character
    sequence = []
    while 1 + 1 == 2:
        char_code = utf8.__read_one_utf8_code_from_stream(sh)
        if char_code == 0xFF:
            raise RegularExpressionException("End of file reached while parsing quoted string.")

        elif char_code == ord("\\"): 
            char_code = snap_backslashed_character.do(sh)
            if char_code == None: 
                raise RegularExpressionException("Unidentified backslash-sequence in quoted string.")

        elif char_code == ord('"'):
            break

        sequence.append(char_code)

    return sequence

