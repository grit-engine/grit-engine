from copy import copy
from quex.frs_py.string_handling import blue_print
db={}

def get_label(StateMachineName, StateIdx, SuccessfulOriginalStateMachineID=None):
    """
        (1) StateIdx != None
            jump label for state machine state entry    
        (2) StateIdx == None:  terminal state
            (1a) SuccessfulOriginalStateMachineID == None: not acceptance terminal state
            (1b) else: acceptance terminal state
    """
    def nice(SM_ID): return repr(SM_ID).replace("L", "")

    if StateIdx != None: 
        return "QUEX_LABEL_%s_ENTRY_%s" % (StateMachineName, nice(StateIdx))
    elif SuccessfulOriginalStateMachineID == None:
        return "QUEX_LABEL_%s_TERMINAL" % StateMachineName
    else:       
        return "QUEX_LABEL_%s_TERMINAL_%s" % (StateMachineName, nice(SuccessfulOriginalStateMachineID))

#________________________________________________________________________________
# C++
#
         
__cpp_function_header = \
"""#include<istream>
#define  QUEX_CHAR_BUFFER_BASED

#if defined QUEX_ANALYSER_STREAM_BASED
#    define QUEX_ANALYSER_FUNC_ARGS     std::istream&  stream
#    define QUEX_ANALYSER_RETURN_TYPE   int
#    define QUEX_CHARACTER_TYPE         int
#    define QUEX_CHARACTER_POSITION   std::istream::pos_type
#    define QUEX_STREAM_GET(character)  (character = stream.get())
#    define QUEX_STREAM_TELL(position)  (position = stream.tellg())
#    define QUEX_STREAM_SEEK(position)  (stream.seekg((position)))
#endif

#if defined QUEX_CHAR_BUFFER_BASED
/* Pass a pointer to a pointer to the analyser, because the last lexing
** position is to be stored. This way the next call to the analyser can
** start where it has stopped before.
*/
#    define QUEX_ANALYSER_FUNC_ARGS     unsigned char** char_pp
#    define QUEX_ANALYSER_RETURN_TYPE   int
#    define QUEX_CHARACTER_TYPE         unsigned char
#    define QUEX_CHARACTER_POSITION   unsigned char*
#    define QUEX_STREAM_GET(character)  character = (**char_pp); ++(*char_pp); 
#    define QUEX_STREAM_TELL(position)  position = *char_pp;
#    define QUEX_STREAM_SEEK(position)  *char_pp = position;
#endif

#if defined QUEX_ANALYSER_UNICODE_VECTOR_BASED
// Each element in the vector contains a unicode character
#    define QUEX_ANALYSER_FUNC_ARGS     std::vector<int>::iterator& it
#    define QUEX_ANALYSER_RETURN_TYPE   int
#    define QUEX_CHARACTER_TYPE         int
#    define QUEX_CHARACTER_POSITION   std::vector<int>::iterator
#    define QUEX_STREAM_GET(character)  character = (*it); ++it; 
#    define QUEX_STREAM_TELL(position)  position = it;
#    define QUEX_STREAM_SEEK(position)  it = position;
#endif

QUEX_ANALYSER_RETURN_TYPE
$$QUEX_ANALYZER_FUNCTION_NAME$$(QUEX_ANALYSER_FUNC_ARGS) {
    int                        last_acceptance = -1;
    QUEX_CHARACTER_POSITION  last_acceptance_input_position = (QUEX_CHARACTER_POSITION)(0x00);
    QUEX_CHARACTER_TYPE        input = (QUEX_CHARACTER_TYPE)(0x00);\n
"""
def __cpp_analyser_function(FunctionName, function_body):   
    txt = __cpp_function_header.replace("$$QUEX_ANALYZER_FUNCTION_NAME$$", 
                                        FunctionName)
    txt += function_body
    txt += "}\n"
    return txt

__cpp_terminal_state_str  = """
    // (*) terminal states
    //
    //     NOTE: 'input' contains the next character already, so there is no
    //            need to read it from the stream again. Thus,
    //
    //              goto QUEX_LABEL_%%STATE_MACHINE_NAME%%_ENTRY_INITIAL_STATE;
    //
    //            at the end of each acceptance terminal state. However:
    //
    //              goto %%INITIAL_STATE_INDEX_LABEL%%;
    // 
    //            However, in cases that the related action contains a 'return' it has
    //            to be sure that the lexical analysis starts at the previous position.
    //            Thus one needs to 'seek-back' first. We write the 'input-get' code directly
    //            after the pattern action, so that the compiler might be able to recognize
    //            a redundancy during optimization.

    // specific terminal states, i.e. related to a 'winner pattern'. this means,
    // that the last input before the pattern matched a complete pattern.
  %%SPECIFIC_TERMINAL_STATES%%

  TERMINAL_GENERAL:
    int tmp = last_acceptance;
    last_acceptance = 0x00;    // reset the last acceptance position for next run
    // jump to last acceptance state
    //     if last acceptance:
    //        -- execute pattern action 
    //        -- goto initial state
    //     else:
    //        -- execute defaul action
    //        -- goto initial state    
    switch( tmp ) {
        %%JUMPS_TO_ACCEPTANCE_STATE%%
        default:
           // no acceptance state    
           %%DEFAULT_ACTION%%
           QUEX_STREAM_GET(input);
           goto QUEX_LABEL_%%STATE_MACHINE_NAME%%_ENTRY_INITIAL_STATE;     
    }
"""

def __cpp_terminal_states(StateMachineName, sm, action_db, DefaultAction):
    
    # -- specific terminal states of patterns (entered from acceptance states)
    txt = ""
    for state_machine_id in action_db.keys():
        txt += "  %s:\n" % get_label("", None, state_machine_id)
        action_code = "    " + action_db[state_machine_id].replace("\n", "\n    ")   
        txt += "    QUEX_STREAM_SEEK(last_acceptance_input_position);"
        txt += action_code + "\n"    
        txt += "    // if action code returns from the function, then the following is meaningless\n"
        if sm.states[sm.init_state_index].transitions().is_empty() == False:
            txt += "    QUEX_STREAM_GET(input);"
        txt += "    goto QUEX_LABEL_%s_ENTRY_INITIAL_STATE;\n" %  StateMachineName

    specific_terminal_states_str = txt

    #  -- general terminal state (entered from non-acceptance state)    
    txt = ""    
    for state_machine_id in action_db.keys():
        txt += "     case %s: goto %s;\n" % \
                (repr(state_machine_id), get_label("", None, state_machine_id))
    jumps_to_acceptance_states_str = txt


    #     -- execute default pattern action 
    #     -- reset character stream to last success             
    #     -- goto initial state 
    txt = blue_print(__cpp_terminal_state_str, 
                     [["%%JUMPS_TO_ACCEPTANCE_STATE%%",    jumps_to_acceptance_states_str],   
                      ["%%SPECIFIC_TERMINAL_STATES%%",     specific_terminal_states_str],
                      ["%%DEFAULT_ACTION%%",               DefaultAction.replace("\n", "        \n")],
                      ["%%STATE_MACHINE_NAME%%",           StateMachineName],
                      ["%%INITIAL_STATE_INDEX_LABEL%%",    get_label(StateMachineName, sm.init_state_index)]])
    return txt
    
#________________________________________________________________________________
# C
#    
db["C"]  = copy(db["C++"])
db["C"]["$/*"] = "/*"
db["C"]["$*/"] = "*/\n"
db["C"]["$function_def"] = "const int\n$$function_name$$(const int input)\n{\n"
db["C"]["$return_true"]  = "return 1;"
db["C"]["$return_false"] = "return 0;"

#________________________________________________________________________________
# Perl
#    
db["Perl"] = copy(db["C"])
db["Perl"]["$function_def"] = "sub $$function_name$$ {\n    input = shift\n"


#________________________________________________________________________________
# Python
#    
def __python_goto_state(UserDefinedStateMachineName, StateIdx, SuccessfulOriginalStateMachineID=None):
    if StateIdx == None: 
        return __python_goto_terminal_state(UserDefinedStateMachineName,  
                                            SuccessfulOriginalStateMachineID)
    txt = "# QUEX_LABEL_%s_ENTRY_%s;\n" % (UserDefinedStateMachineName,
                                         repr(StateIdx).replace("L",""))
    return txt + "return %s" % repr(StateIdx)
         
def __python_goto_terminal_state(UserDefinedStateMachineName, SuccessfulOriginalStateMachineID=None):
    if SuccessfulOriginalStateMachineID == None:
        txt = "# goto QUEX_LABEL_%s_TERMINAL;\n" % UserDefinedStateMachineName
    else:       
        txt = "# goto QUEX_LABEL_%s_TERMINAL_%s;\n" % (UserDefinedStateMachineName,
                                                       repr(SuccessfulOriginalStateMachineID).replace("L",""))
    return txt + "return -1"    

def __python_note_acceptance(SuccessfulOriginalStateMachineID):
    if SuccessfulOriginalStateMachineID != None:
        txt =  "# last_acceptance = %s\n" % SuccessfulOriginalStateMachineID
        txt += "# last_acceptance_input_position = stream.tell()\n"
    else:
        txt = ""    
    return txt



def replace_keywords(program_txt, LanguageDB, NoIndentF):
    """Replaces pseudo-code keywords with keywords of the given language."""

    txt = blue_print(program_txt, LanguageDB.items())

    if NoIndentF == False:
        # delete the last newline, to prevent additional indentation
        if txt[-1] == "\n": txt = txt[:-1]
        # indent by four spaces
        # (if this happens in recursively called functions nested indented blocks
        #  are correctly indented, see NumberSet::get_condition_code() for example)     
        txt = txt.replace("\n", "\n    ") + "\n"
    
    return txt          
