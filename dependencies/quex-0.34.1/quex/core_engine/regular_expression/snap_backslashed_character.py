from copy           import deepcopy
from quex.exception import  RegularExpressionException
from StringIO       import StringIO


backslashed_character_db = { 
        # inside string "..." and outside 
        'a': ord('\a'),   'b': ord('\b'), 'f': ord('\f'),   'n': ord('\n'),
        'r': ord('\r'),   't': ord('\t'), 'v': ord('\v'),   '\\': ord('\\'), '"': ord('"'),
        # only ouside of string
        '+': ord('+'), '*': ord('*'), '?': ord('?'), '/': ord('/'), 
        '|': ord('|'), '$': ord('$'), '^': ord('^'), '-': ord('-'), 
        '[': ord('['), ']': ord(']'),    
        '(': ord('('), ')': ord(')'),  
        '{': ord('{'), '}': ord('}'), 
}
        
def do(sh, ReducedSetOfBackslashedCharactersF=False):
    """All backslashed characters shall enter this function. In particular 
       backslashed characters appear in:
        
             "$50"     -- quoted strings
             [a-zA-Z]  -- character sets
             for       -- lonestanding characters 
    
       x = string containing characters after 'the backslash'
       i = position of the backslash in the given string

       ReducedSetOfBackslashedCharactersF indicates whether we are outside of a quoted
       string (lonestanding characters, sets, etc.) or inside a string. Inside a quoted
       string there are different rules, because not all control characters need to be
       considered.

       RETURNS: UCS code of the interpreted character,
                index of first element after the treated characters in the string
    """
    assert sh.__class__ == StringIO or sh.__class__ == file
    assert type(ReducedSetOfBackslashedCharactersF) == bool 

    if ReducedSetOfBackslashedCharactersF:
        backslashed_character_list = [ 'a', 'b', 'f', 'n', 'r', 't', 'v', '\\', '"' ]
    else:
        backslashed_character_list = backslashed_character_db.keys()

    tmp = sh.read(1)
    if tmp == "":
        raise RegularExpressionException("End of file while parsing backslash sequence.")

    if   tmp in backslashed_character_list: return backslashed_character_db[tmp]
    elif tmp.isdigit():                     sh.seek(-1,1); return __parse_octal_number(sh, 5)
    elif tmp == 'x':                        return __parse_hex_number(sh, 2)
    elif tmp == 'X':                        return __parse_hex_number(sh, 4)
    elif tmp == 'U':                        return __parse_hex_number(sh, 6)
    else:
        raise RegularExpressionException("Backslashed '%s' is unknown to quex." % tmp)

def __parse_octal_number(sh, MaxL):
    return __parse_base_number(sh, MaxL, 
                               DigitSet   = "01234567",
                               Base       = 8,
                               NumberName = "octal")

def __parse_hex_number(sh, MaxL):
    return __parse_base_number(sh, MaxL, 
                               DigitSet   = "0123456789abcdefABCDEF",
                               Base       = 16,
                               NumberName = "hexadecimal")

def __parse_base_number(sh, MaxL, DigitSet, Base, NumberName):
    """MaxL = Maximum length of number to be parsed.
    """
    number_str = ""
    tmp        = sh.read(1)
    while tmp != "" and tmp in DigitSet:
        number_str += tmp
        if len(number_str) == MaxL: break
        tmp = sh.read(1)
    else:
        if tmp != "": sh.seek(-1,1)
        
    if number_str == "": 
        raise RegularExpressionException("Missing %s number." % NumberName)

    return long(number_str, Base)
