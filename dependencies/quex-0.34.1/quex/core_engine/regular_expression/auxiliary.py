from quex.exception import RegularExpressionException
from quex.core_engine.state_machine.core import StateMachine

__debug_recursion_depth  = -1
__debug_output_enabled_f = False # True / False 

def __snap_until(stream, ClosingDelimiter, OpeningDelimiter=None):
     """Cuts the first letters of the utf8_string until an un-backslashed
        Delimiter occurs.
     """
     cut_string = ""  
     backslash_f = False
     open_bracket_n = 1 
     while 1 + 1 == 2:
        letter = stream.read(1)
        if letter == "": 
            raise RegularExpressionException("Unable to find closing delimiter '%s'" % ClosingDelimiter)

        cut_string += letter    

        if letter == "\\": 
            backslash_f = not backslash_f       
            continue
            
        elif letter == ClosingDelimiter and not backslash_f: 
            if open_bracket_n == 1: cut_string = cut_string[:-1]; break
            open_bracket_n -= 1

        elif letter == OpeningDelimiter and not backslash_f: 
            # NOTE: if OpeningDelimiter == None, then this can never be the case!
            open_bracket_n += 1

        # if a backslash would have appeared, we would have 'continue'd (see above)
        backslash_f = False    
     else:
        raise RegularExpressionException("Unable to find closing delimiter '%s'" % ClosingDelimiter)
   
     return cut_string

def __debug_print(msg, msg2="", msg3=""):
    global __debug_recursion_depth
    if type(msg2) != str: msg2 = repr(msg2)
    if type(msg3) != str: msg3 = repr(msg3)
    txt = "##" + "  " * __debug_recursion_depth + msg + " " + msg2 + " " + msg3
    txt = txt.replace("\n", "\n    " + "  " * __debug_recursion_depth)
    if __debug_output_enabled_f: print txt
    
def __debug_exit(result, stream):
    global __debug_recursion_depth
    __debug_recursion_depth -= 1

    if __debug_output_enabled_f: 
        pos = stream.tell()
        txt = stream.read()
        stream.seek(pos)    
        __debug_print("##exit: [%s], remainder = \"%s\"" % (repr(result), txt))
        
    return result

def __debug_entry(function_name, stream):
    global __debug_recursion_depth
    __debug_recursion_depth += 1

    if __debug_output_enabled_f: 
        pos = stream.tell()
        txt = stream.read()
        stream.seek(pos)    
        __debug_print("##entry: %s, remainder = \"%s\"" % (function_name, txt))

