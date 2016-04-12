from copy import copy
from quex.frs_py.string_handling import blue_print

#________________________________________________________________________________
# Python
#    
def __transition(CurrentStateIdx, StateIdx, 
                 TargetStateIdx, BackwardLexingF=False, DeadEndStateDB={}):
    # NOTE: This is a very rudimental implementation of the __goto_state, see the cpp implementation
    #       for a complete implementation.
    UserDefinedStateMachineName = "unimportant"
    if StateIdx == None: 
        return __goto_terminal_state(UserDefinedStateMachineName,  
                                     TargetStateIdx)  # specific terminal state

    txt = "# QUEX_LABEL_%s_ENTRY_%s;\n" % (UserDefinedStateMachineName,
                                           repr(StateIdx).replace("L",""))
    return txt + "return %s" % repr(StateIdx)
        
         
def __goto_terminal_state(UserDefinedStateMachineName, SuccessfulOriginalStateMachineID=None):
    if SuccessfulOriginalStateMachineID == None:
        txt = "# goto QUEX_LABEL_%s_TERMINAL;\n" % UserDefinedStateMachineName
    else:       
        txt = "# goto QUEX_LABEL_%s_TERMINAL_%s;\n" % (UserDefinedStateMachineName,
                                                       repr(SuccessfulOriginalStateMachineID).replace("L",""))
    return txt + "return -1"    

def __note_acceptance(SuccessfulOriginalStateMachineID, LanguageDB, BackwardLexingF=False, BackwardInputPositionDetectionF=False):
    if SuccessfulOriginalStateMachineID != None:
        txt =  "# last_acceptance = %s\n" % SuccessfulOriginalStateMachineID
        txt += "# last_acceptance_input_position = stream.tell()\n"
    else:
        txt = ""    
    return txt

def __label_definition(LabelName):
    return "# case " + LabelName + ":"

def __state_drop_out_code(StateMachineName, CurrentStateIdx, BackwardLexingF,
                          CurrentStateIsAcceptanceF = None,
                          OriginList                = None,
                          LanguageDB                = None,
                          DropOutTargetStateID      = -1):
    return ""

def __get_if_in_character_set(ValueList):
    assert type(ValueList) == list
    assert len(ValueList) > 0

    txt = "if input in ["
    for value in ValueList:
        txt += "%i, " % value
    txt += "]:\n"

    return txt

def __get_if_in_interval(TriggerSet):
    assert TriggerSet.__class__.__name__ == "Interval"
    assert TriggerSet.size() >= 2

    if TriggerSet.size() == 2:
        return "if input == %i or input == %i:\n" % (TriggerSet.begin, TriggerSet.end - 1)
    else:
        return "if input >= %i and input < %i:\n" % (TriggerSet.begin, TriggerSet.end)

