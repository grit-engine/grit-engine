from   quex.frs_py.file_in import *
import quex.lexer_mode     as     lexer_mode
from   quex.token_id_maker import TokenInfo
from   quex.input.setup    import setup as Setup
from   quex.input.ucs_db_parser  import ucs_property_db
from   quex.core_engine.utf8 import __read_one_utf8_code_from_stream
from   quex.core_engine.generator.action_info import *

LanguageDB = Setup.language_db


def parse(fh, CodeFragmentName, 
          ErrorOnFailureF=True, AllowBriefTokenSenderF=True):
    """RETURNS: An object of class UserCodeFragment containing
                line number, filename, and the code fragment.

                None in case of failure.
    """
    assert Setup.__class__.__name__ == "something"
    assert type(ErrorOnFailureF)        == bool
    assert type(AllowBriefTokenSenderF) == bool

    skip_whitespace(fh)
    position = fh.tell()

    word = fh.read(2)
    if len(word) >= 1 and word[0] == "{":
        fh.seek(-1, 1) # unput the second character
        return __parse_normal(fh, CodeFragmentName)

    elif AllowBriefTokenSenderF and word == "=>":
        return __parse_brief_token_sender(fh)

    elif not ErrorOnFailureF:
        fh.seek(-2,1)
        return None
    else:
        error_msg("missing code fragment after %s definition." % CodeFragmentName, fh)

def __parse_normal(fh, code_fragment_name):
    line_n = get_current_line_info_number(fh) + 1
    code   = read_until_closing_bracket(fh, "{", "}")
    return UserCodeFragment(code, fh.name, line_n, LanguageDB, AddReferenceCodeF=True)

def __parse_brief_token_sender(fh):
    # shorthand for { self.send(TKN_SOMETHING); RETURN; }
    
    position = fh.tell()
    line_n   = get_current_line_info_number(fh) + 1
    try: 
        skip_whitespace(fh)
        position = fh.tell()

        code = __parse_token_id_specification_by_character_code(fh)
        if code != "": return UserCodeFragment(code, fh.name, line_n, LanguageDB, AddReferenceCodeF=True)

        identifier, arg_list_str = __parse_function_call(fh)
        if identifier in ["GOTO", "GOSUB", "GOUP"]:
            code = __create_mode_transition_and_token_sender(fh, identifier, arg_list_str)
        else:
            code = __create_token_sender_by_token_name(fh, identifier, arg_list_str)

        if code != "": 
            return UserCodeFragment(code, fh.name, line_n, LanguageDB, AddReferenceCodeF=True)
        else:
            return None

    except EndOfStreamException:
        fh.seek(position)
        error_msg("End of file reached while parsing token shortcut.", fh)

def read_character_code(fh):
    # NOTE: This function is tested with the regeression test for feature request 2251359.
    #       See directory $QUEX_PATH/TEST/2251359.
    pos = fh.tell()
    
    start = fh.read(1)
    if start == "":  
        seek(pos); return -1

    elif start == "'": 
        # read an utf-8 char an get the token-id
        # Example: '+'
        character_code = __read_one_utf8_code_from_stream(fh)
        if character_code == 0xFF:
            error_msg("Missing utf8-character for definition of character code by character.", fh)

        elif fh.read(1) != '\'':
            error_msg("Missing closing ' for definition of character code by character.", fh)

        return character_code

    if start == "U":
        if fh.read(1) != "C": seek(pos); return -1
        # read Unicode Name 
        # Example: UC MATHEMATICAL_MONOSPACE_DIGIT_FIVE
        skip_whitespace(fh)
        ucs_name = read_identifier(fh)
        if ucs_name == "": seek(pos); return -1
        # Get the character set related to the given name. Note, the size of the set
        # is supposed to be one.
        character_code = ucs_property_db.get_character_set("Name", ucs_name)
        if type(character_code) in [str, unicode]:
            error_msg("%s does not identify a known unicode character." % ucs_name, fh) 
        if type(character_code) not in [int, long]:
            error_msg("%s relates to more than one character in unicode database." % ucs_name, fh) 
        return character_code

    second = fh.read(1)
    if start == "0" and second.isdigit() == False:
        base = second
        if base not in ["x", "o", "b"]: 
            error_msg("Number base '0%s' is unknown, please use '0x' for hexidecimal,\n" % base + \
                      "'0o' for octal, or '0b' for binary.", fh)
        number_txt = read_integer(fh)
        if number_txt == "":
            error_msg("Missing integer number after '0%s'" % base, fh)
        try: 
            if   base == "x": character_code = int("0x" + number_txt, 16) 
            elif base == "o": character_code = int(number_txt, 8) 
            elif base == "b": 
                character_code = 0
                for letter in number_txt:
                    character_code = character_code << 1
                    if   letter == "1":   character_code += 1
                    elif letter != "0":
                        error_msg("Letter '%s' not permitted in binary number (something start with '0b')" % letter, fh)
            else:
                # A normal integer number (starting with '0' though)
                character_code = int(base + number_text)
        except:
            error_msg("The string '%s' is not appropriate for number base '0%s'." % (number_txt, base), fh)

        return character_code

    elif start.isdigit():
        fh.seek(-2, 1) # undo 'start' and 'second'
        # All that remains is that it is a 'normal' integer
        number_txt = read_integer(fh)

        if number_txt == "": fh.seek(pos); return -1
        
        try:    return int(number_txt)
        except: error_msg("The string '%s' is not appropriate for number base '10'." % number_txt, fh)

    else:
        # Try to interpret it as something else ...
        fh.seek(pos); return -1               

def __parse_function_call(fh):
    position = fh.tell()
    try:
        skip_whitespace(fh)
        identifier = read_identifier(fh)
        skip_whitespace(fh)

        tmp = fh.read(1)
        if tmp not in ["(", ";"]:
            error_msg("Missing '(' or ';' after '%s'." % identifier, fh)
        if tmp == ";":
            return identifier, ""  # No argument list, ";" arrived immediately

        arg_list_str = read_until_closing_bracket(fh, "(", ")")
        verify_next_word(fh, ";")
            
        return identifier, arg_list_str

    except EndOfStreamException:
        fh.seek(position)
        error_msg("End of file reached while parsing token shortcut.", fh)

def __parse_token_id_specification_by_character_code(fh):
    character_code = read_character_code(fh)
    if character_code == -1: return ""

    verify_next_word(fh, ";")
    prefix_less_token_name = "UCS_0x%06X" % character_code
    token_id_str = Setup.input_token_id_prefix + prefix_less_token_name
    lexer_mode.token_id_db[prefix_less_token_name] = \
            TokenInfo(prefix_less_token_name, character_code, None, fh.name, get_current_line_info_number(fh)) 
    txt  = "#ifdef QUEX_OPTION_TOKEN_SENDING_VIA_QUEUE\n"
    txt += "self.send(%s); return;\n" % token_id_str
    txt += "#else\n"
    txt += "self.send(%s); return %s;\n" % (token_id_str, token_id_str)
    txt += "#endif\n"
    return txt

def __create_token_sender_by_token_name(fh, TokenName, ArgListStr):
    assert type(ArgListStr) == str

    # after 'send' the token queue is filled and one can safely return
    if TokenName.find(Setup.input_token_id_prefix) != 0:
        error_msg("token identifier does not begin with token prefix '%s'\n" % Setup.input_token_id_prefix + \
                  "found: '%s'" % TokenName, fh)

    # occasionally add token id automatically to database
    prefix_less_TokenName = TokenName[len(Setup.input_token_id_prefix):]
    if not lexer_mode.token_id_db.has_key(prefix_less_TokenName):
        msg = "Token id '%s' defined implicitly." % TokenName
        if TokenName in lexer_mode.token_id_db.keys():
            msg += "\nNOTE: '%s' has been defined in a token { ... } section!" % \
                   (Setup.input_token_id_prefix + TokenName)
            msg += "\nNote, that tokens in the token { ... } section are automatically prefixed."
        error_msg(msg, fh, DontExitF=True)

        lexer_mode.token_id_db[prefix_less_TokenName] = \
                TokenInfo(prefix_less_TokenName, None, None, fh.name, get_current_line_info_number(fh)) 

    tail = ArgListStr
    if tail != "": tail = ", " + tail
    txt  = "#ifdef QUEX_OPTION_TOKEN_SENDING_VIA_QUEUE\n"
    txt += "self.send(%s%s); return;\n" % (TokenName, tail)
    txt += "#else\n"
    txt += "self.send(%s); return %s;\n" % (ArgListStr, TokenName)
    txt += "#endif\n"

    return txt


def __create_mode_transition_and_token_sender(fh, Command, ArgListStr):
    assert Command in ["GOTO", "GOSUB", "GOUP"]
    assert type(ArgListStr) == str

    arg_list = ArgListStr.split(",")
    arg_list = filter(lambda arg: arg != "", arg_list)
    L = len(arg_list)
    target_mode = None
    token_name  = None
    tail        = []
    if Command in ["GOTO", "GOSUB"]:
        if L < 1: 
            error_msg("The %s mode short cut requires at least one argument: The target mode." % Command, fh)
        target_mode = arg_list[0]
        if L > 1: token_name = arg_list[1]
        if L > 2: tail = arg_list[2:]
    else: # Command == "GOUP"
        if L > 0: token_name = arg_list[0]
        if L > 1: tail = arg_list[1:]

    mode_change_str = { "GOTO":  lambda Mode: "self << " + Mode + ";\n",
                        "GOSUB": lambda Mode: "self.push_mode(" + Mode + ");\n",
                        "GOUP":  lambda Mode: "self.pop_mode();\n"
                      }[Command](target_mode)

    tail_str = ""
    for element in tail:
        tail_str += ", " + element

    if token_name != None: send_str = "self.send(%s%s); "% (token_name, tail_str)
    else:                  send_str = "" 

    txt  = mode_change_str
    txt += "#ifdef QUEX_OPTION_TOKEN_SENDING_VIA_QUEUE\n"
    txt += send_str + "return;\n" 
    txt += "#else\n"
    txt += send_str + "return %s;\n" % token_name
    txt += "#endif\n"
    return txt

