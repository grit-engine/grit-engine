from copy import copy
from quex.frs_py.file_in         import is_identifier_start, is_identifier_continue
from quex.frs_py.string_handling import blue_print

import quex.core_engine.state_machine.index as index
#

def __nice(SM_ID): 
    return repr(SM_ID).replace("L", "")

#________________________________________________________________________________
# C++
#

__header_definitions_txt = """
#include <quex/code_base/template/Analyser>
#include <quex/code_base/buffer/Buffer>

#ifdef CONTINUE
#   undef CONTINUE
#endif
#define CONTINUE  $$GOTO_START_PREPARATION$$
"""

def __header_definitions(LanguageDB):

    txt = __header_definitions_txt
    txt = txt.replace("$$GOTO_START_PREPARATION$$", LanguageDB["$goto"]("$re-start"))
    return txt

def __local_variable_definitions(VariableInfoList):
    txt = ""
    L = max(map(lambda info: len(info[0]), VariableInfoList))
    for info in VariableInfoList:
        if len(info) > 3: 
            if info[3] != 0:
                type = info[0]
                name = info[1] + "[%s]" % repr(info[3])
                value = "/* uninitialized */"
            else:
                type = info[0] + "*"
                name = info[1] 
                value = "= 0x0"
        else:
            type  = info[0]
            name  = info[1] 
            value = "= " + info[2]
        txt += "    %s%s %s %s;\n" % (type, " " * (L-len(type)), name, value)
    return txt
         
__function_signature = """
__QUEX_SETTING_ANALYSER_FUNCTION_RETURN_TYPE  
$$QUEX_ANALYZER_STRUCT_NAME$$_$$STATE_MACHINE_NAME$$_analyser_function(QuexAnalyser* me) 
{
    /* NOTE: Different modes correspond to different analyser functions. The analyser*/
    /*       functions are all located inside the main class as static functions. That*/
    /*       means, they are something like 'globals'. They receive a pointer to the */
    /*       lexical analyser, since static member do not have access to the 'this' pointer.*/
#   if defined (__QUEX_SETTING_PLAIN_C)
#      define self (*me)
#   else
       using namespace quex;
       QUEX_LEXER_CLASS& self = *((QUEX_LEXER_CLASS*)me);
#   endif
"""

comment_on_post_context_position_init_str = """
    /* Post context positions do not have to be reset or initialized. If a state
     * is reached which is associated with 'end of post context' it is clear what
     * post context is meant. This results from the ways the state machine is 
     * constructed. A post context positions live time looks like the following:
     *
     * (1)   unitialized (don't care)
     * (1.b) on buffer reload it may, or may not be adapted (don't care)
     * (2)   when a post context begin state is passed, the it is **SET** (now: take care)
     * (2.b) on buffer reload it **is adapted**.
     * (3)   when a terminal state of the post context is reached (which can only be reached
     *       for that particular post context, then the post context position is used
     *       to reset the input position.                                              */
"""


def __analyser_function(StateMachineName, EngineClassName, StandAloneEngineF,
                        function_body, PostConditionedStateMachineID_List, PreConditionIDList,
                        ModeNameList=[], InitialStateIndex=-1, LanguageDB=None):   
    """EngineClassName = name of the structure that contains the engine state.
                         if a mode of a complete quex environment is created, this
                         is the mode name. otherwise, any name can be chosen. 
       StandAloneEngineF = False if a mode for a quex engine is to be created. True
                           if a stand-alone lexical engine is required (without the
                           complete mode-handling framework of quex).
        
       NOTE: If a stand-alone lexer is requested, then there are two functions that are
             created additionally: 

               'EngineClassName'_init(EngineClassName* me,
                                      QUEX_CHARACTER_TYPE StartInputPosition);

                     This function has to be called before starting the lexing process.
                     See the unit tests for examples.
               
               'EngineClassName'_do(EngineClassName* me);
                     
                     This function does a lexical analysis from the current position as
                     it is stored in 'me'.
    """              
    txt = ""

    local_variable_list = []
    signature = __function_signature

    if not StandAloneEngineF: 
        L = max(map(lambda name: len(name), ModeNameList))
        for name in ModeNameList:
            local_variable_list.append(["quex::QuexMode&", name + " " * (L- len(name)), 
                                        "QUEX_LEXER_CLASS::" + name]) 

    txt  = "#include <quex/code_base/temporary_macros_on>\n"
    txt += signature
    txt  = txt.replace("$$STATE_MACHINE_NAME$$", StateMachineName) 

    txt += "    " + LanguageDB["$comment"]("me = pointer to state of the lexical analyser") + "\n"

    PostContextN = len(PostConditionedStateMachineID_List)
    local_variable_list.extend(
            [ ["QUEX_GOTO_LABEL_TYPE",         "last_acceptance",                "QUEX_GOTO_TERMINAL_LABEL_INIT_VALUE"],
              ["QUEX_CHARACTER_POSITION_TYPE", "last_acceptance_input_position", "(QUEX_CHARACTER_TYPE*)(0x00)"],
              ["QUEX_CHARACTER_POSITION_TYPE", "post_context_start_position",    "(QUEX_CHARACTER_TYPE*)(0x00)", 
                                                                                 PostContextN],
              ["const size_t",                 "PostContextStartPositionN",      "(size_t)" + repr(PostContextN)],
              ["QUEX_CHARACTER_TYPE",          "input",                          "(QUEX_CHARACTER_TYPE)(0x00)"]
            ])
              
    # -- pre-condition fulfillment flags                
    for pre_context_sm_id in PreConditionIDList:
        local_variable_list.append(["int", "pre_context_%s_fulfilled_f" % __nice(pre_context_sm_id), "0"])

    txt += __local_variable_definitions(local_variable_list)
    txt += comment_on_post_context_position_init_str
    txt += "#if    defined(QUEX_OPTION_AUTOMATIC_ANALYSIS_CONTINUATION_ON_MODE_CHANGE) \\\n"
    txt += "    || defined(QUEX_OPTION_ASSERTS)\n"
    txt += "    me->DEBUG_analyser_function_at_entry = me->current_analyser_function;\n"
    txt += "#endif\n"

    txt += LanguageDB["$label-def"]("$start")

    # -- entry to the actual function body
    txt += "    " + LanguageDB["$mark-lexeme-start"] + "\n"
    txt += "    QuexBuffer_undo_terminating_zero_for_lexeme(&me->buffer);\n";
    
    txt += function_body

    # -- prevent the warning 'unused variable'
    txt += "\n"
    txt += "    /* prevent compiler warning 'unused variable': use variables once in a part of the code*/\n"
    txt += "    /* that is never reached (and deleted by the compiler anyway).*/\n"
    txt += "    if( 0 == 1 ) {\n"
    txt += "        int unused = 0;\n"
    for mode_name in ModeNameList:
        txt += "        unused = unused + %s.id;\n" % mode_name
    ## This was once we did not know ... if there was a goto to the initial state or not.
    ## txt += "        goto %s;\n" % label.get(StateMachineName, InitialStateIndex)

    txt += "    }\n"

    txt += "}\n"

    # -- the name of the game
    txt = txt.replace("$$QUEX_ANALYZER_STRUCT_NAME$$", EngineClassName)

    txt += "#include <quex/code_base/temporary_macros_off>\n"
    return txt

__terminal_state_str  = """
  /* (*) Terminal states _______________________________________________________*/
  /**/
  /* Acceptance terminal states, i.e. the 'winner patterns'. This means*/
  /* that the last input dropped out of a state where the longest matching*/
  /* pattern was according to the terminal state. The terminal states are */
  /* numbered after the pattern id.*/
  /**/
#define Lexeme       (me->buffer._lexeme_start_p)
#define LexemeBegin  (me->buffer._lexeme_start_p)
#define LexemeEnd    (me->buffer._input_p)
#define LexemeL      (size_t)(me->buffer._input_p - me->buffer._lexeme_start_p)
$$SPECIFIC_TERMINAL_STATES$$

$$TERMINAL_END_OF_STREAM-DEF$$
$$END_OF_STREAM_ACTION$$
#ifdef __QUEX_OPTION_ANALYSER_RETURN_TYPE_IS_VOID
        return /*__QUEX_TOKEN_ID_TERMINATION*/;
#else
        return __QUEX_TOKEN_ID_TERMINATION;
#endif

$$TERMINAL_DEFAULT-DEF$$
$$DEFAULT_ACTION$$
        $$GOTO_START_PREPARATION$$

#undef Lexeme
#undef LexemeBegin
#undef LexemeEnd
#undef LexemeL
#ifndef __QUEX_OPTION_USE_COMPUTED_GOTOS
__TERMINAL_ROUTER: {
        /*  if last_acceptance => goto correspondent acceptance terminal state*/
        /*  else               => execute defaul action*/
        switch( last_acceptance ) {
$$JUMPS_TO_ACCEPTANCE_STATE$$
            default: $$TERMINAL_DEFAULT-GOTO$$; /* nothing matched */
        }
    }
#endif /* __QUEX_OPTION_USE_COMPUTED_GOTOS */
"""

__on_continue_reentry_preparation_str = """
  
$$REENTRY_PREPARATION$$
    /* (*) Common point for **restarting** lexical analysis.
     *     at each time when CONTINUE is called at the end of a pattern. */
    last_acceptance = QUEX_GOTO_TERMINAL_LABEL_INIT_VALUE;
$$DELETE_PRE_CONDITION_FULLFILLED_FLAGS$$
$$COMMENT_ON_POST_CONTEXT_INITIALIZATION$$
    /*  If a mode change happened, then the function must first return and
     *  indicate that another mode function is to be called. At this point, 
     *  we to force a 'return' on a mode change. 
     *
     *  Pseudo Code: if( previous_mode != current_mode ) {
     *                   return 0;
     *               }
     *
     *  When the analyzer returns, the caller function has to watch if a mode change
     *  occured. If not it can call this function again.                               */
#if    defined(QUEX_OPTION_AUTOMATIC_ANALYSIS_CONTINUATION_ON_MODE_CHANGE) \
    || defined(QUEX_OPTION_ASSERTS)
    if( me->DEBUG_analyser_function_at_entry != me->current_analyser_function ) 
#endif
    { 
#if defined(QUEX_OPTION_AUTOMATIC_ANALYSIS_CONTINUATION_ON_MODE_CHANGE)
#   ifdef __QUEX_OPTION_ANALYSER_RETURN_TYPE_IS_VOID
       return /*__QUEX_TOKEN_ID_UNINITIALIZED*/;
#   else
       return __QUEX_TOKEN_ID_UNINITIALIZED;
#   endif
#elif defined(QUEX_OPTION_ASSERTS)
       QUEX_ERROR_EXIT("Mode change without immediate return from the lexical analyser.");
#endif
    }

    $$GOTO_START$$
"""

def __adorn_action_code(action_info, SMD, SupportBeginOfLineF, IndentationOffset=4): 
    indentation = " " * IndentationOffset 
    txt = ""
    # TODO: There could be a differenciation between a pattern that contains
    #       newline at the end, and those that do not. Then, there need not
    #       be a conditional question.
    if SupportBeginOfLineF:
        txt += indentation + "QuexBuffer_store_last_character_of_lexeme_for_next_run(&me->buffer);\n"

    if action_info.action().require_terminating_zero_f():
        txt += indentation + "QuexBuffer_set_terminating_zero_for_lexeme(&me->buffer);\n"

    code_str = action_info.action().get_code()

    txt += indentation + "{\n"
    txt += indentation + "    " + code_str.replace("\n", "\n        ") + "\n"  
    txt += indentation + "}\n"

    return txt

def get_terminal_code(state_machine_id, SMD, pattern_action_info, SupportBeginOfLineF, LanguageDB):
    state_machine                  = SMD.sm()
    DirectlyReachedTerminalID_List = SMD.directly_reached_terminal_id_list()

    txt = ""
    state_machine_id_str = __nice(state_machine_id)
    state_machine        = pattern_action_info.pattern_state_machine()
    #
    action_code = __adorn_action_code(pattern_action_info, SMD, SupportBeginOfLineF)
        
    # (*) The 'normal' terminal state can also be reached by the terminal
    #     router and, thus, **must** restore the acceptance input position. This is so, 
    #     because when the 'goto last_acceptance' is triggered the 'last_acceptance'
    #     may lay backwards and needs to be restored.
    txt += LanguageDB["$label-def"]("$terminal", state_machine_id) + "\n"

    # (1) Retrieving the input position for the next run
    #     NOTE: The different scenarios differ in the way they can 'offer' an entry
    #           to the terminal without restoring the input position. This 'direct'
    #           entry is useful for direct transitions to a terminal where there
    #           acceptance position is clear.
    if state_machine.core().post_context_backward_input_position_detector_sm() != None:
        # Pseudo Ambiguous Post Contexts:
        # -- require that the end of the core pattern is to be searched! One 
        #    cannot simply restore some stored input position.
        # -- The pseudo-ambiguous post condition is translated into a 'normal'
        #    pattern. However, after a match a backward detection of the end
        #    of the core pattern is done. Here, we first need to go to the point
        #    where the 'normal' pattern ended, then we can do a backward detection.
        txt += "    " + LanguageDB["$input/seek_position"]("last_acceptance_input_position") + "\n"
        txt += LanguageDB["$label-def"]("$terminal-direct", state_machine_id) + "\n"
        txt += "    PAPC_input_postion_backward_detector_%s(me);\n" % \
               __nice(state_machine.core().post_context_backward_input_position_detector_sm_id())

    elif state_machine.core().post_context_id() != -1L: 
        post_condition_index = SMD.get_post_context_index(state_machine_id)
        # Post Contexted Patterns:
        txt += LanguageDB["$label-def"]("$terminal-direct", state_machine_id) + "\n"
        # -- have a dedicated register from where they store the end of the core pattern.
        variable = "post_context_start_position[%s]" % __nice(post_condition_index) 
        txt += "    " + LanguageDB["$comment"]("post context index '%s' == state machine '%s'" % \
                                               (__nice(post_condition_index), __nice(state_machine_id))) + "\n"
        txt += "    " + LanguageDB["$input/seek_position"](variable) + "\n"

    else:
        # Normal Acceptance:
        # -- only restore the input position
        txt += "    " + LanguageDB["$input/seek_position"]("last_acceptance_input_position") + "\n"
        txt += LanguageDB["$label-def"]("$terminal-direct", state_machine_id) + "\n"



    # -- paste the action code that correponds to the pattern   
    txt += action_code + "\n"    
    txt += "    " + LanguageDB["$goto"]("$re-start") + "\n" 
    txt += "\n"

    return txt

def __terminal_states(SMD, action_db, DefaultAction, EndOfStreamAction, 
                      SupportBeginOfLineF, PreConditionIDList, LanguageDB):
    """NOTE: During backward-lexing, for a pre-condition, there is not need for terminal
             states, since only the flag 'pre-condition fulfilled is raised.
    """      
    assert SMD.__class__.__name__ == "StateMachineDecorator"
    sm = SMD.sm()
    PostConditionedStateMachineID_List = SMD.post_contexted_sm_id_list()
    DirectlyReachedTerminalID_List     = SMD.directly_reached_terminal_id_list()

    # (*) specific terminal states of patterns (entered from acceptance states)
    txt = ""
    for state_machine_id, pattern_action_info in action_db.items():
        txt += get_terminal_code(state_machine_id, SMD, pattern_action_info, SupportBeginOfLineF, LanguageDB)
    specific_terminal_states_str = txt

    # (*) general terminal state (entered from non-acceptance state)    
    txt = ""    
    for state_machine_id in action_db.keys():
        txt += "            case %s: " % repr(state_machine_id).replace("L", "")
        txt += LanguageDB["$goto"]("$terminal", state_machine_id) + "\n"
    jumps_to_acceptance_states_str = txt

    # (*) preparation of the reentry without return:
    #     delete all pre-condition fullfilled flags
    txt = ""
    for pre_context_sm_id in PreConditionIDList:
        txt += "    " + LanguageDB["$assignment"]("pre_context_%s_fulfilled_f" % __nice(pre_context_sm_id), 0)
    delete_pre_context_flags_str = txt

    #  -- execute default pattern action 
    #  -- goto initial state    
    end_of_stream_code_action_str = __adorn_action_code(EndOfStreamAction, SMD, SupportBeginOfLineF,
                                                        IndentationOffset=16)
    # -- DEFAULT ACTION: Under 'normal' circumstances the default action is simply to be executed
    #                    since the 'get_forward()' incremented the 'current' pointer.
    #                    HOWEVER, when end of file has been reached the 'current' pointer has to
    #                    be reset so that the initial state can drop out on the buffer limit code
    #                    and then transit to the end of file action.
    # NOTE: It is possible that 'miss' happens after a chain of characters appeared. In any case the input
    #       pointer must be setup right after the lexeme start. This way, the lexer becomes a new chance as
    #       soon as possible.
    default_action_str  = "me->buffer._input_p = me->buffer._lexeme_start_p;\n"
    default_action_str += LanguageDB["$if EOF"] + "\n"
    default_action_str += "    " + LanguageDB["$comment"]("Next increment will stop on EOF character.") + "\n"
    default_action_str += LanguageDB["$endif"] + "\n"
    default_action_str += LanguageDB["$else"] + "\n"
    default_action_str += "    " + LanguageDB["$comment"]("Step over nomatching character") + "\n"
    default_action_str += "    " + LanguageDB["$input/increment"] + "\n"
    default_action_str += LanguageDB["$endif"] + "\n"
    default_action_str += __adorn_action_code(DefaultAction, SMD, SupportBeginOfLineF,
                                              IndentationOffset=16)

    # -- routing to states via switch statement
    #    (note, the gcc computed goto is implement, too)
    txt = ""
    for state_index, state in sm.states.items():
        if state.transitions().is_empty(): continue
        txt += "            "
        txt += "case %i: " % int(state_index) + LanguageDB["$goto"]("$input", state_index) + "\n"

    if sm.core().pre_context_sm() != None:
        for state_index, state in sm.core().pre_context_sm().states.items():
            if state.transitions().is_empty(): continue
            txt += "            "
            txt += "case %i: " % int(state_index) + LanguageDB["$goto"]("$input", state_index) + "\n"

    switch_cases_drop_out_back_router_str = txt

    if PreConditionIDList == []: precondition_involved_f = "0"
    else:                        precondition_involved_f = "1"

    txt = blue_print(__terminal_state_str, 
                     [["$$JUMPS_TO_ACCEPTANCE_STATE$$",    jumps_to_acceptance_states_str],   
                      ["$$SPECIFIC_TERMINAL_STATES$$",     specific_terminal_states_str],
                      ["$$DEFAULT_ACTION$$",               default_action_str],
                      ["$$END_OF_STREAM_ACTION$$",         end_of_stream_code_action_str],
                      ["$$TERMINAL_END_OF_STREAM-DEF$$",   LanguageDB["$label-def"]("$terminal-EOF")],
                      ["$$TERMINAL_DEFAULT-DEF$$",         LanguageDB["$label-def"]("$terminal-DEFAULT")],
                      ["$$TERMINAL_GENERAL-DEF$$",         LanguageDB["$label-def"]("$terminal-general", False)],
                      ["$$TERMINAL_DEFAULT-GOTO$$",        LanguageDB["$goto"]("$terminal-DEFAULT")],
                      ["$$STATE_MACHINE_NAME$$",           SMD.name()],
                      ["$$GOTO_START_PREPARATION$$",       LanguageDB["$goto"]("$re-start")],
                      ])

    txt += blue_print(__on_continue_reentry_preparation_str,
                      [["$$REENTRY_PREPARATION$$",                    LanguageDB["$label-def"]("$re-start")],
                       ["$$DELETE_PRE_CONDITION_FULLFILLED_FLAGS$$",  delete_pre_context_flags_str],
                       ["$$GOTO_START$$",                             LanguageDB["$goto"]("$start")],
                       ["$$COMMENT_ON_POST_CONTEXT_INITIALIZATION$$", comment_on_post_context_position_init_str],
                       ])

    return txt
    

def __frame_of_all(Code, HeaderFile, LexerClassName):
    return "#include \"%s\"\n" % HeaderFile                 + \
           "#if ! defined(__QUEX_SETTING_PLAIN_C)\n"        + \
           "namespace quex {\n"                             + \
           "#endif\n"                                       + \
           "#define QUEX_LEXER_CLASS %s\n" % LexerClassName + \
           Code                                             + \
           "#if ! defined(__QUEX_SETTING_PLAIN_C)\n"        + \
           "} // namespace quex\n"                          + \
           "#endif\n" 

def __get_if_in_character_set(ValueList):
    assert type(ValueList) == list
    assert len(ValueList) > 0
    txt = "if( "
    for value in ValueList:
        txt += "input == %i || " % value

    txt = txt[:-3] + ") {\n"
    return txt

def __get_if_in_interval(TriggerSet):
    assert TriggerSet.__class__.__name__ == "Interval"
    assert TriggerSet.size() >= 2

    if TriggerSet.size() == 2:
        return "if( input == %i || input == %i ) {\n" % (TriggerSet.begin, TriggerSet.end - 1)
    else:
        return "if( input >= %i && input < %i ) {\n" % (TriggerSet.begin, TriggerSet.end)


def __require_terminating_zero_preparation(LanguageDB, CodeStr):
    CommentDelimiterList = LanguageDB["$comment-delimiters"]
    assert type(CommentDelimiterList) == list
    ObjectName = "Lexeme"

    for delimiter_info in CommentDelimiterList:
        assert type(delimiter_info) == list, "Argument 'CommentDelimiters' must be of type [[]]"
        assert len(delimiter_info) == 3, \
               "Elements of argument CommentDelimiters must be arrays with three elements:\n" + \
               "start of comment, end of comment, replacement string for comment.\n" + \
               "received: " + repr(delimiter_info)

    txt = CodeStr
    L       = len(txt)
    LO      = len(ObjectName)
    found_i = -1
    while 1 + 1 == 2:
        # TODO: Implement the skip_whitespace() function for more general treatment of Comment
        #       delimiters. Quotes for strings '"" shall then also be treate like comments.
        found_i = txt.find(ObjectName, found_i + 1)

        if found_i == -1: return False

        # Note: The variable must be named 'exactly' like the given name. 'xLexeme' or 'Lexemey'
        #       shall not trigger a treatment of 'Lexeme'.
        if     (found_i == 0      or not is_identifier_start(txt[found_i - 1]))     \
           and (found_i == L - LO or not is_identifier_continue(txt[found_i + LO])): 
               return True
