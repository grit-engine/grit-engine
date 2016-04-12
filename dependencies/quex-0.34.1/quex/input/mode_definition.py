from   quex.frs_py.file_in          import *
from   quex.token_id_maker          import TokenInfo
from   quex.exception               import RegularExpressionException
import quex.lexer_mode                as lexer_mode
import quex.input.regular_expression  as regular_expression
import quex.input.code_fragment       as code_fragment
from   quex.core_engine.generator.action_info import CodeFragment
from   quex.core_engine.generator.skip_code import create_skip_code, create_skip_range_code
import quex.core_engine.state_machine.index as index
from   quex.core_engine.state_machine.core  import StateMachine
import quex.core_engine.regular_expression.snap_character_string as snap_character_string
from   quex.input.setup    import setup as Setup

def parse(fh, Setup):
    # NOTE: Catching of EOF happens in caller: parse_section(...)

    skip_whitespace(fh)
    mode_name = read_identifier(fh)
    if mode_name == "":
        error_msg("missing identifier at beginning of mode definition.", fh)

    # NOTE: constructor does register this mode in the mode_db
    new_mode  = lexer_mode.LexMode(mode_name, fh.name, get_current_line_info_number(fh))

    # (*) inherited modes / options
    skip_whitespace(fh)
    dummy = fh.read(1)
    if dummy not in [":", "{"]:
        error_msg("missing ':' or '{' after mode '%s'" % mode_name, fh)

    if dummy == ":":
        parse_mode_option_list(new_mode, fh)

    # (*) read in pattern-action pairs and events
    while parse_mode_element(new_mode, fh): 
        pass

    # (*) check for modes w/o pattern definitions
    if not new_mode.has_event_handler() and new_mode.own_matches() == {}:
        if new_mode.options["inheritable"] != "only":
            new_mode.options["inheritable"] = "only"
            error_msg("Mode without pattern and event handlers needs to be 'inheritable only'.\n" + \
                      "<inheritable: only> has been added automatically.", fh,  DontExitF=True)

def parse_mode_option_list(new_mode, fh):
    position = fh.tell()
    try:  
        # ':' => inherited modes/options follow
        skip_whitespace(fh)

        # (*) base modes 
        base_modes, i = read_until_letter(fh, ["{", "<"], Verbose=1)
        new_mode.base_modes = split(base_modes)

        if i != 1: return
        fh.seek(-1, 1)

        # (*) options
        while parse_mode_option(fh, new_mode):
            pass

    except EndOfStreamException:
        fh.seek(position)
        error_msg("End of file reached while options of mode '%s'." % mode_name, fh)

def parse_mode_option(fh, new_mode):
    skip_whitespace(fh)

    # (*) base modes 
    if fh.read(1) != "<": return False

    skip_whitespace(fh)

    identifier = read_identifier(fh).strip()

    if identifier == "":  error_msg("missing identifer after start of mode option '<'", fh)
    skip_whitespace(fh)
    if fh.read(1) != ":": error_msg("missing ':' after option name '%s'" % identifier, fh)
    skip_whitespace(fh)

    if identifier == "skip":
        # A skipper 'eats' characters at the beginning of a pattern that belong
        # to a specified set of characters. A useful application is most probably
        # the whitespace skipper '[ \t\n]'. The skipper definition allows quex to
        # implement a very effective way to skip these regions.
        pattern_str, trigger_set = regular_expression.parse_character_set(fh, PatternStringF=True)
        skip_whitespace(fh)

        if fh.read(1) != ">":
            error_msg("missing closing '>' for mode option '%s'." % identifier, fh)

        if trigger_set.is_empty():
            error_msg("Empty trigger set for skipper." % identifier, fh)

        # TriggerSet skipping is implemented the following way: As soon as one element of the 
        # trigger set appears, the state machine enters the 'trigger set skipper section'.
        opener_sm = StateMachine()
        opener_sm.add_transition(opener_sm.init_state_index, trigger_set, AcceptanceF=True)
            
        action = CodeFragment(create_skip_code(trigger_set))
 
        # Enter the skipper as if the opener pattern was a normal pattern and the 'skipper' is the action.
        new_mode.add_match(pattern_str, action, opener_sm)

        return True

    elif identifier == "skip_range":
        # A non-nesting skipper can contain a full fledged regular expression as opener,
        # since it only effects the trigger. Not so the nested range skipper-see below.

        # -- opener
        skip_whitespace(fh)
        opener_str, opener_sm = regular_expression.parse(fh)
        skip_whitespace(fh)

        # -- closer
        if fh.read(1) != "\"":
            error_msg("closing pattern for skip_range can only be a string and must start with a quote like \".", fh)
        closer_sequence = snap_character_string.get_character_code_sequence(fh)
        skip_whitespace(fh)
        if fh.read(1) != ">":
            error_msg("missing closing '>' for mode option '%s'" % identifier, fh)

        action = CodeFragment(create_skip_range_code(closer_sequence))

        # Enter the skipper as if the opener pattern was a normal pattern and the 'skipper' is the action.
        new_mode.add_match(opener_str, action, opener_sm)
        return True
        
    elif identifier == "skip_nesting_range":
        error_msg("skip_nesting_range is not yet supported.", fh)

    else:
        value, i = read_until_letter(fh, [">"], Verbose=1)
        if i != 0:
            error_msg("missing closing '>' for mode option '%s'" % identifier, fh)

        value = value.strip()

    # Does the specified option actually exist?
    if not lexer_mode.mode_option_info_db.has_key(identifier):
        error_msg("tried to set option '%s' which does not exist!\n" % identifier + \
                  "options are %s" % repr(lexer_mode.mode_option_info_db.keys()), fh)

    # Is the option of the appropriate value?
    option_info = lexer_mode.mode_option_info_db[identifier]
    if option_info.type != "list" and value not in option_info.domain:
        error_msg("Tried to set value '%s' for option '%s'. " % (Value, Option) + \
                  "Though, possible \n" + \
                  "for this option are %s" % repr(oi.domain), fh)

    # Finally, set the option
    new_mode.add_option(identifier, value)

    return True

def parse_mode_element(new_mode, fh):
    """Returns: False, if a closing '}' has been found.
                True, else.
    """
    position = fh.tell()
    try:
        description = "Pattern or event handler name.\n" + \
                      "Missing closing '}' for end of mode"

        skip_whitespace(fh)
        # NOTE: Do not use 'read_word' since we need to continue directly after
        #       whitespace, if a regular expression is to be parsed.
        position = fh.tell()

        word = read_until_whitespace(fh)
        if word == "}":
            return False

        # -- check for 'on_entry', 'on_exit', ...
        result = check_for_event_specification(word, fh, new_mode)
        if result == True: 
            return True # all work has been done in check_for_event_specification()
        else:
            fh.seek(position)
            description = "start of mode element: regular expression"
            pattern, pattern_state_machine = regular_expression.parse(fh)

            if new_mode.has_pattern(pattern):
                previous = new_mode.get_match_object(pattern)
                error_msg("Pattern has been defined twice.", fh, DontExitF=True)
                error_msg("First defined here.", 
                         previous.action().filename, previous.action().line_n)


        position    = fh.tell()
        description = "start of mode element: code fragment for '%s'" % pattern

        parse_action_code(new_mode, fh, pattern, pattern_state_machine)

    except EndOfStreamException:
        fh.seek(position)
        error_msg("End of file reached while parsing %s." % description, fh)

    return True

def parse_action_code(new_mode, fh, pattern, pattern_state_machine):

    position = fh.tell()
    try:
        skip_whitespace(fh)
        position = fh.tell()
            
        code_obj = code_fragment.parse(fh, "regular expression", ErrorOnFailureF=False) 
        if code_obj != None:
            new_mode.add_match(pattern, code_obj, pattern_state_machine)
            return

        fh.seek(position)
        word = read_until_letter(fh, [";"])
        if word == "PRIORITY-MARK":
            # This mark 'lowers' the priority of a pattern to the priority of the current
            # pattern index (important for inherited patterns, that have higher precedence).
            # The parser already constructed a state machine for the pattern that is to
            # be assigned a new priority. Since, this machine is not used, let us just
            # use its id.
            fh.seek(-1, 1)
            verify_next_word(fh, ";", Comment="Since quex version 0.33.5 this is required.")
            new_mode.add_match_priority(pattern, pattern_state_machine, pattern_state_machine.get_id(), fh)

        elif word == "DELETION":
            # This mark deletes any pattern that was inherited with the same 'name'
            fh.seek(-1, 1)
            verify_next_word(fh, ";", Comment="Since quex version 0.33.5 this is required.")
            new_mode.add_match_deletion(pattern, pattern_state_machine, fh)
            
        else:
            error_msg("missing token '{', 'PRIORITY-MARK', 'DELETION', or '=>' after '%s'.\n" % pattern + \
                      "found: '%s'. Note, that since quex version 0.33.5 it is required to add a ';'\n" % word + \
                      "to the commands PRIORITY-MARK and DELETION.", fh)


    except EndOfStreamException:
        fh.seek(position)
        error_msg("End of file reached while parsing action code for pattern.", fh)

def check_for_event_specification(word, fh, new_mode):

    if word == "on_entry":
        # Event: enter into mode
        new_mode.on_entry = code_fragment.parse(fh, "%s::on_entry event handler" % new_mode.name)
        return True
    
    elif word == "on_exit":
        # Event: exit from mode
        new_mode.on_exit = code_fragment.parse(fh, "%s::on_exit event handler" % new_mode.name)
        return True

    elif word == "on_match":
        # Event: exit from mode
        new_mode.on_match = code_fragment.parse(fh, "%s::on_match event handler" % new_mode.name)
        return True

    elif  word == "on_indentation":
        # Event: start of indentation, 
        #        first non-whitespace after whitespace
        new_mode.on_indentation = code_fragment.parse(fh, "%s::on_indentation event handler" % new_mode.name)
        return True

    elif word == "on_failure" or word == "<<FAIL>>":
        # Event: No pattern matched for current position.
        # NOTE: See 'on_end_of_stream' comments.
        new_mode.on_failure = code_fragment.parse(fh, "%s::on_failure event handler" % new_mode.name)
        return True

    elif word == "on_end_of_stream" or word == "<<EOF>>": 
        # Event: End of data stream / end of file
        # NOTE: The regular expression parser relies on <<EOF>> and <<FAIL>>. So those
        #       patterns are entered here, even if later versions of quex might dismiss
        #       those rule deefinitions in favor of consistent event handlers.
        new_mode.on_end_of_stream = code_fragment.parse(fh, "%s::on_end_of_stream event handler" % new_mode.name)
        return True

    elif len(word) >= 3 and word[:3] == "on_":    
        error_msg("Unknown event handler '%s'. Known event handlers are:\n\n" % word + \
                  "on_entry, on_exit, on_indentation, on_end_of_stream, on_failure. on_match\n\n" + \
                  "Note, that any pattern starting with 'on_' is considered an event handler.\n" + \
                  "use double quotes to bracket patterns that start with 'on_'.", fh)

    # word was not an event specification 
    return False

