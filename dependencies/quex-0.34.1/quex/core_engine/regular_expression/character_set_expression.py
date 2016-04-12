# GRAMMAR:
#
# set_expression: 
#                 [: set_term :]
#                 traditional character set
#                 \P '{' propperty string '}'
#
# set_term:
#                 "alnum" 
#                 "alpha" 
#                 "blank" 
#                 "cntrl" 
#                 "digit" 
#                 "graph" 
#                 "lower" 
#                 "print" 
#                 "punct" 
#                 "space" 
#                 "upper" 
#                 "xdigit"
#                 "union"        '(' set_term [ ',' set_term ]+ ')'
#                 "intersection" '(' set_term [ ',' set_term ]+ ')'
#                 "difference"   '(' set_term [ ',' set_term ]+ ')'
#                 "inverse"      '(' set_term ')'
#                 set_expression
# 
import quex.core_engine.regular_expression.traditional_character_set as traditional_character_set
import quex.core_engine.regular_expression.property                  as property
import quex.core_engine.regular_expression.auxiliary                 as aux
#
from quex.core_engine.state_machine.core import StateMachine
from quex.exception                      import RegularExpressionException
from quex.frs_py.file_in                 import read_until_letter, \
                                                read_until_non_letter, \
                                                skip_whitespace
from quex.core_engine.regular_expression.auxiliary import __snap_until, \
                                                          __debug_entry, \
                                                          __debug_exit

special_character_set_db = {
        # The closing ']' is to trigger the end of the traditional character set
        "alnum":  traditional_character_set.do_string("a-zA-Z0-9]"),
        "alpha":  traditional_character_set.do_string("a-zA-Z]"),
        "blank":  traditional_character_set.do_string(" \\t]"),
        "cntrl":  traditional_character_set.do_string("\\x00-\\x1F\\x7F]"), 
        "digit":  traditional_character_set.do_string("0-9]"),
        "graph":  traditional_character_set.do_string("\\x21-\\x7E]"),
        "lower":  traditional_character_set.do_string("a-z]"),
        "print":  traditional_character_set.do_string("\\x20-\\x7E]"), 
        "punct":  traditional_character_set.do_string("!\"#$%&'()*+,-./:;?@[\\]_`{|}~\\\\]"),
        "space":  traditional_character_set.do_string(" \\t\\r\\n]"),
        "upper":  traditional_character_set.do_string("A-Z]"),
        "xdigit": traditional_character_set.do_string("a-fA-F0-9]"),
}

def do(stream):
    trigger_set = snap_set_expression(stream)

    if trigger_set == None: 
        raise RegularExpressionException("Regular Expression: character_set_expression called for something\n" + \
                                         "that does not start with '[:', '[' or '\\P'")
    if trigger_set.is_empty():
        raise RegularExpressionException("Regular Expression: Character set expression results in empty set.")

    # Create state machine that triggers with the trigger set to SUCCESS
    # NOTE: The default for the ELSE transition is FAIL.
    sm = StateMachine()
    sm.add_transition(sm.init_state_index, trigger_set, AcceptanceF=True)

    return __debug_exit(sm, stream)

def snap_set_expression(stream):
    assert     stream.__class__.__name__ == "StringIO" \
            or stream.__class__.__name__ == "file"

    __debug_entry("set_expression", stream)

    result = snap_property_set(stream)
    if result != None: return result

    x = stream.read(2)
    if   x == "[:":
        result = snap_set_term(stream)
        skip_whitespace(stream)
        x = stream.read(2)
        if x != ":]":
            raise RegularExpressionException("Missing closing ':]' for character set expression.\n" + \
                                             "found: '%s'" % x)
    elif x[0] == "[":
        stream.seek(-1, 1)
        result = traditional_character_set.do(stream)   
    elif x == "\\P": 
        stream.seek(-2, 1)
        result = property.do(stream)
    elif x == "\\N": 
        stream.seek(-2, 1)
        result = property.do_shortcut(stream, "N", "na") # UCS Property: Name
    elif x == "\\G": 
        stream.seek(-2, 1)
        result = property.do_shortcut(stream, "G", "gc") # UCS Property: General_Category
    else:
        result = None

    return __debug_exit(result, stream)

def snap_property_set(stream):
    position = stream.tell()
    x = stream.read(2)
    if x == "\\P": 
        stream.seek(position)
        return property.do(stream)
    elif x == "\\N": 
        stream.seek(position)
        return property.do_shortcut(stream, "N", "na") # UCS Property: Name
    elif x == "\\G": 
        stream.seek(position)
        return property.do_shortcut(stream, "G", "gc") # UCS Property: General_Category
    else:
        stream.seek(position)
        return None

def snap_set_term(stream):
    __debug_entry("set_term", stream)    

    skip_whitespace(stream)
    position = stream.tell()

    # if there is no following '(', then enter the 'snap_expression' block below
    try:    
        word = read_until_non_letter(stream)
        stream.seek(-1, 1)  # putback the non-letter
    except: 
        word = "not a valid word"

    word = word.strip()

    if word in [ "union", "intersection", "difference", "inverse"]: 
        set_list = snap_set_list(stream, word)
        # if an error occurs during set_list parsing, an exception is thrown about syntax error

        L      = len(set_list)
        result = set_list[0]

        if word == "inverse":
            # The inverse of multiple sets, is to be the inverse of the union of these sets.
            if L > 1:
                for set in set_list[1:]:
                    result.unite_with(set)
            result = result.inverse()
            return __debug_exit(result, stream)

        if L < 2:
            raise RegularExpressionException("Regular Expression: A %s operation needs at least\n" % word + \
                                             "two sets to operate on them.")
            
        if   word == "union":
            for set in set_list[1:]:
                result.unite_with(set)
        elif word == "intersection":
            for set in set_list[1:]:
                result.intersect_with(set)
        elif word == "difference":
            for set in set_list[1:]:
                result.subtract(set)

    elif word in special_character_set_db.keys():
        result = special_character_set_db[word]

    else:
        # try to snap an expression out of it
        stream.seek(position)
        result = snap_set_expression(stream)

    return __debug_exit(result, stream)

def __snap_word(stream):
    try:    the_word = read_until_letter(stream, ["("]) 
    except: 
        raise RegularExpressionException("Missing opening bracket.")
    stream.seek(-1,1)
    return the_word.strip()

def snap_set_list(stream, set_operation_name):
    __debug_entry("set_list", stream)

    skip_whitespace(stream)
    if stream.read(1) != "(": 
        raise RegularExpressionException("Missing opening bracket '%s' operation." % set_operation_name)

    set_list = []
    while 1 + 1 == 2:
        skip_whitespace(stream)
        result = snap_set_term(stream)
        if result == None: 
            raise RegularExpressionException("Missing set expression list after '%s' operation." % set_operation_name)
        set_list.append(result)
        skip_whitespace(stream)
        tmp = stream.read(1)
        if tmp != ",": 
            if tmp != ")":
                stream.seek(-1, 1)
                raise RegularExpressionException("Missing closing ')' after after '%s' operation." % set_operation_name)
            return __debug_exit(set_list, stream)


   
