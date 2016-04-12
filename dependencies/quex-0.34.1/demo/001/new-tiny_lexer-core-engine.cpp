    /* Information about what pattern 'comes' from what mode in the inheritance tree.
     * 
     * [1] pattern, [2] dominating mode, [3] dominating inheritance level, [4] pattern index
     * 
     * (STRING_READER)
     *  {P_BACKSLASHED_STRING_DELIMITER} STRING_READER  0 00052
     *  {P_BACKSLASHED_BACKSLASH} STRING_READER  0 00054
     *  {P_STRING_DELIMITER} STRING_READER  0 00056
     * 
     * 
     * (PROGRAM)
     *  \"{\" PROGRAM  0 00022
     *  \"}\" PROGRAM  0 00025
     *  \"=\" PROGRAM  0 00028
     *  \"struct\" PROGRAM  0 00031
     *  if PROGRAM  0 00034
     *  \";\" PROGRAM  0 00037
     *  [a-z]+ PROGRAM  0 00044
     *  {P_NUMBER} PROGRAM  0 00045
     *  [ \t\n] PROGRAM  0 00048
     *  {P_STRING_DELIMITER} PROGRAM  0 00050
     * 
     * 
     * 
     */
#include "tiny_lexer"
#if ! defined(__QUEX_SETTING_PLAIN_C)
namespace quex {
#endif
#define QUEX_LEXER_CLASS tiny_lexer

#include <quex/code_base/template/Analyser>
#include <quex/code_base/buffer/Buffer>

#ifdef CONTINUE
#   undef CONTINUE
#endif
#define CONTINUE  goto __REENTRY_PREPARATION;
#include <quex/code_base/temporary_macros_on>

__QUEX_SETTING_ANALYSER_FUNCTION_RETURN_TYPE  
tiny_lexer_STRING_READER_analyser_function(QuexAnalyser* me) 
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
    /* me = pointer to state of the lexical analyser */
    quex::QuexMode&              STRING_READER = QUEX_LEXER_CLASS::STRING_READER;
    quex::QuexMode&              PROGRAM       = QUEX_LEXER_CLASS::PROGRAM;
    QUEX_GOTO_LABEL_TYPE         last_acceptance = QUEX_GOTO_TERMINAL_LABEL_INIT_VALUE;
    QUEX_CHARACTER_POSITION_TYPE last_acceptance_input_position = (QUEX_CHARACTER_TYPE*)(0x00);
    QUEX_CHARACTER_POSITION_TYPE* post_context_start_position = 0x0;
    const size_t                 PostContextStartPositionN = (size_t)0;
    QUEX_CHARACTER_TYPE          input = (QUEX_CHARACTER_TYPE)(0x00);

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
#if    defined(QUEX_OPTION_AUTOMATIC_ANALYSIS_CONTINUATION_ON_MODE_CHANGE) \
    || defined(QUEX_OPTION_ASSERTS)
    me->DEBUG_analyser_function_at_entry = me->current_analyser_function;
#endif
__REENTRY:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: __REENTRY");
    QuexBuffer_mark_lexeme_start(&me->buffer);
    QuexBuffer_undo_terminating_zero_for_lexeme(&me->buffer);
    /* state machine */
    /* init-state = 176L
     * 00176() <~ (52, 147), (54, 153), (56, 157)
     *       == '"' ==> 00178
     *       == '\' ==> 00177
     *       <no epsilon>
     * 00177() <~ (52, 148), (54, 154)
     *       == '"' ==> 00180
     *       == '\' ==> 00179
     *       <no epsilon>
     * 00179(A, S) <~ (54, 155, A, S)
     *       <no epsilon>
     * 00180(A, S) <~ (52, 149, A, S)
     *       <no epsilon>
     * 00178(A, S) <~ (56, 158, A, S)
     *       <no epsilon>
     * 
     */
STATE_176:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_176");

    input = QuexBuffer_input_get(&me->buffer);
    if( input < 35) {
        if( input != 34) {
            goto STATE_176_DROP_OUT;    /* [-oo, '!'] */
        } else {
            QuexBuffer_input_p_increment(&me->buffer);
            goto TERMINAL_56_DIRECT;    /* '"' */
        }
    } else {
        if( input == 92) {
            goto STATE_177;    /* '\' */
        } else {
            goto STATE_176_DROP_OUT_DIRECT;    /* ['#', '['] */
        }
    }

STATE_176_DROP_OUT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_176_DROP_OUT");
    if( input != QUEX_SETTING_BUFFER_LIMIT_CODE ) {
STATE_176_DROP_OUT_DIRECT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_176_DROP_OUT_DIRECT");
        QUEX_GOTO_last_acceptance();

    }

    if( QuexBuffer_is_end_of_file(&me->buffer) ) {
        /* NO CHECK 'last_acceptance != -1' --- first state can **never** be an acceptance state */
        goto TERMINAL_END_OF_STREAM;
    }
        QUEX_DEBUG_PRINT(&me->buffer, "FORWARD_BUFFER_RELOAD");
    if( QuexAnalyser_buffer_reload_forward(&me->buffer, &last_acceptance_input_position,
                                           post_context_start_position, PostContextStartPositionN) ) {
       goto STATE_176_INPUT;
    }

    QUEX_DEBUG_PRINT(&me->buffer, "BUFFER_RELOAD_FAILED");
    QUEX_GOTO_last_acceptance();


STATE_176_INPUT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_176_INPUT");
    QuexBuffer_input_p_increment(&me->buffer);
    goto STATE_176;
STATE_177:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_177");

STATE_177_INPUT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_177_INPUT");

    QuexBuffer_input_p_increment(&me->buffer);
    input = QuexBuffer_input_get(&me->buffer);
    if( input < 35) {
        if( input != 34) {
            goto STATE_177_DROP_OUT;    /* [-oo, '!'] */
        } else {
            QuexBuffer_input_p_increment(&me->buffer);
            goto TERMINAL_52_DIRECT;    /* '"' */
        }
    } else {
        if( input == 92) {
            QuexBuffer_input_p_increment(&me->buffer);
            goto TERMINAL_54_DIRECT;    /* '\' */
        } else {
            goto STATE_177_DROP_OUT_DIRECT;    /* ['#', '['] */
        }
    }

STATE_177_DROP_OUT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_177_DROP_OUT");
    if( input != QUEX_SETTING_BUFFER_LIMIT_CODE ) {
STATE_177_DROP_OUT_DIRECT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_177_DROP_OUT_DIRECT");
        QUEX_GOTO_last_acceptance();

    }

        QUEX_DEBUG_PRINT(&me->buffer, "FORWARD_BUFFER_RELOAD");
    if( QuexAnalyser_buffer_reload_forward(&me->buffer, &last_acceptance_input_position,
                                           post_context_start_position, PostContextStartPositionN) ) {
       goto STATE_177_INPUT;
    }

    QUEX_DEBUG_PRINT(&me->buffer, "BUFFER_RELOAD_FAILED");
    QUEX_GOTO_last_acceptance();




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
TERMINAL_56:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: TERMINAL_56");

    QuexBuffer_seek_memory_adr(&me->buffer, last_acceptance_input_position);

TERMINAL_56_DIRECT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: TERMINAL_56_DIRECT");

    {
        
        #line 1 ""
        {
        self.counter.__shift_end_values_to_start_values();
        self.counter.count_NoNewline(1);
        
        #line 83 "simple.qx"
        self << PROGRAM; RETURN; 
#line 220 "tiny_lexer-core-engine.cpp"
        
        }
#line 223 "tiny_lexer-core-engine.cpp"
        
    }

    goto __REENTRY_PREPARATION;

TERMINAL_52:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: TERMINAL_52");

    QuexBuffer_seek_memory_adr(&me->buffer, last_acceptance_input_position);

TERMINAL_52_DIRECT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: TERMINAL_52_DIRECT");

    QuexBuffer_set_terminating_zero_for_lexeme(&me->buffer);
    {
        
        #line 1 ""
        {
        self.counter.__shift_end_values_to_start_values();
        self.counter.count_NoNewline(2);
        
        #line 80 "simple.qx"
        self.accumulator.add(Lexeme); 
#line 247 "tiny_lexer-core-engine.cpp"
        
        }
#line 250 "tiny_lexer-core-engine.cpp"
        
    }

    goto __REENTRY_PREPARATION;

TERMINAL_54:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: TERMINAL_54");

    QuexBuffer_seek_memory_adr(&me->buffer, last_acceptance_input_position);

TERMINAL_54_DIRECT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: TERMINAL_54_DIRECT");

    QuexBuffer_set_terminating_zero_for_lexeme(&me->buffer);
    {
        
        #line 1 ""
        {
        self.counter.__shift_end_values_to_start_values();
        self.counter.count_NoNewline(2);
        
        #line 81 "simple.qx"
        self.accumulator.add(Lexeme); 
#line 274 "tiny_lexer-core-engine.cpp"
        
        }
#line 277 "tiny_lexer-core-engine.cpp"
        
    }

    goto __REENTRY_PREPARATION;



TERMINAL_END_OF_STREAM:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: TERMINAL_END_OF_STREAM");

                {
                    
        #line 1 ""
        {
        self.counter.__shift_end_values_to_start_values();
        
        }
#line 295 "tiny_lexer-core-engine.cpp"
        
                }

#ifdef __QUEX_OPTION_ANALYSER_RETURN_TYPE_IS_VOID
        return /*__QUEX_TOKEN_ID_TERMINATION*/;
#else
        return __QUEX_TOKEN_ID_TERMINATION;
#endif

TERMINAL_DEFAULT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: TERMINAL_DEFAULT");

me->buffer._input_p = me->buffer._lexeme_start_p;
if( QuexBuffer_is_end_of_file(&me->buffer) ) {

    /* Next increment will stop on EOF character. */
}

else {
    /* Step over nomatching character */    QuexBuffer_input_p_increment(&me->buffer);
}

                QuexBuffer_set_terminating_zero_for_lexeme(&me->buffer);
                {
                    
        #line 1 ""
        {
        self.counter.__shift_end_values_to_start_values();
        self.counter.count(Lexeme, LexemeEnd);
        
        #line 86 "simple.qx"
        self.accumulator.add(Lexeme);
            
#line 329 "tiny_lexer-core-engine.cpp"
        
        }
#line 332 "tiny_lexer-core-engine.cpp"
        
                }

        goto __REENTRY_PREPARATION;

#undef Lexeme
#undef LexemeBegin
#undef LexemeEnd
#undef LexemeL
#ifndef __QUEX_OPTION_USE_COMPUTED_GOTOS
__TERMINAL_ROUTER: {
        /*  if last_acceptance => goto correspondent acceptance terminal state*/
        /*  else               => execute defaul action*/
        switch( last_acceptance ) {
            case 56: goto TERMINAL_56;
            case 52: goto TERMINAL_52;
            case 54: goto TERMINAL_54;

            default: goto TERMINAL_DEFAULT;; /* nothing matched */
        }
    }
#endif /* __QUEX_OPTION_USE_COMPUTED_GOTOS */

  
__REENTRY_PREPARATION:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: __REENTRY_PREPARATION");

    /* (*) Common point for **restarting** lexical analysis.
     *     at each time when CONTINUE is called at the end of a pattern. */
    last_acceptance = QUEX_GOTO_TERMINAL_LABEL_INIT_VALUE;


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
#if    defined(QUEX_OPTION_AUTOMATIC_ANALYSIS_CONTINUATION_ON_MODE_CHANGE)     || defined(QUEX_OPTION_ASSERTS)
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

    goto __REENTRY;

    /* prevent compiler warning 'unused variable': use variables once in a part of the code*/
    /* that is never reached (and deleted by the compiler anyway).*/
    if( 0 == 1 ) {
        int unused = 0;
        unused = unused + STRING_READER.id;
        unused = unused + PROGRAM.id;
    }
}
#include <quex/code_base/temporary_macros_off>

#include <quex/code_base/template/Analyser>
#include <quex/code_base/buffer/Buffer>

#ifdef CONTINUE
#   undef CONTINUE
#endif
#define CONTINUE  goto __REENTRY_PREPARATION;
#include <quex/code_base/temporary_macros_on>

__QUEX_SETTING_ANALYSER_FUNCTION_RETURN_TYPE  
tiny_lexer_PROGRAM_analyser_function(QuexAnalyser* me) 
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
    /* me = pointer to state of the lexical analyser */
    quex::QuexMode&              STRING_READER = QUEX_LEXER_CLASS::STRING_READER;
    quex::QuexMode&              PROGRAM       = QUEX_LEXER_CLASS::PROGRAM;
    QUEX_GOTO_LABEL_TYPE         last_acceptance = QUEX_GOTO_TERMINAL_LABEL_INIT_VALUE;
    QUEX_CHARACTER_POSITION_TYPE last_acceptance_input_position = (QUEX_CHARACTER_TYPE*)(0x00);
    QUEX_CHARACTER_POSITION_TYPE* post_context_start_position = 0x0;
    const size_t                 PostContextStartPositionN = (size_t)0;
    QUEX_CHARACTER_TYPE          input = (QUEX_CHARACTER_TYPE)(0x00);

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
#if    defined(QUEX_OPTION_AUTOMATIC_ANALYSIS_CONTINUATION_ON_MODE_CHANGE) \
    || defined(QUEX_OPTION_ASSERTS)
    me->DEBUG_analyser_function_at_entry = me->current_analyser_function;
#endif
__REENTRY:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: __REENTRY");
    QuexBuffer_mark_lexeme_start(&me->buffer);
    QuexBuffer_undo_terminating_zero_for_lexeme(&me->buffer);
    /* state machine */
    /* init-state = 236L
     * 00236() <~ (22, 63), (25, 69), (28, 75), (31, 91), (34, 104), (37, 111), (44, 128), (45, 132), (48, 138), (50, 142)
     *       == ['\t', '\n'], ' ' ==> 00243
     *       == '"' ==> 00244
     *       == ['0', '9'] ==> 00246
     *       == ';' ==> 00241
     *       == '=' ==> 00240
     *       == ['a', 'h'], ['j', 'r'], ['t', 'z'] ==> 00245
     *       == 'i' ==> 00239
     *       == 's' ==> 00238
     *       == '{' ==> 00242
     *       == '}' ==> 00237
     *       <no epsilon>
     * 00237(A, S) <~ (25, 70, A, S)
     *       <no epsilon>
     * 00238(A, S) <~ (44, 129, A, S), (31, 92)
     *       == ['a', 's'], ['u', 'z'] ==> 00245
     *       == 't' ==> 00248
     *       <no epsilon>
     * 00248(A, S) <~ (44, 129, A, S), (31, 93)
     *       == ['a', 'q'], ['s', 'z'] ==> 00245
     *       == 'r' ==> 00249
     *       <no epsilon>
     * 00249(A, S) <~ (44, 129, A, S), (31, 94)
     *       == ['a', 't'], ['v', 'z'] ==> 00245
     *       == 'u' ==> 00250
     *       <no epsilon>
     * 00250(A, S) <~ (44, 129, A, S), (31, 95)
     *       == ['a', 'b'], ['d', 'z'] ==> 00245
     *       == 'c' ==> 00251
     *       <no epsilon>
     * 00251(A, S) <~ (44, 129, A, S), (31, 96)
     *       == ['a', 's'], ['u', 'z'] ==> 00245
     *       == 't' ==> 00252
     *       <no epsilon>
     * 00252(A, S) <~ (31, 97, A, S)
     *       == ['a', 'z'] ==> 00245
     *       <no epsilon>
     * 00245(A, S) <~ (44, 129, A, S)
     *       == ['a', 'z'] ==> 00245
     *       <no epsilon>
     * 00239(A, S) <~ (44, 129, A, S), (34, 105)
     *       == ['a', 'e'], ['g', 'z'] ==> 00245
     *       == 'f' ==> 00247
     *       <no epsilon>
     * 00247(A, S) <~ (34, 106, A, S)
     *       == ['a', 'z'] ==> 00245
     *       <no epsilon>
     * 00240(A, S) <~ (28, 76, A, S)
     *       <no epsilon>
     * 00241(A, S) <~ (37, 112, A, S)
     *       <no epsilon>
     * 00242(A, S) <~ (22, 64, A, S)
     *       <no epsilon>
     * 00243(A, S) <~ (48, 139, A, S)
     *       <no epsilon>
     * 00244(A, S) <~ (50, 143, A, S)
     *       <no epsilon>
     * 00246(A, S) <~ (45, 133, A, S)
     *       == ['0', '9'] ==> 00246
     *       <no epsilon>
     * 
     */
STATE_236:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_236");

    input = QuexBuffer_input_get(&me->buffer);
    if( input < 61) {
        if( input < 34) {
            if( input == 9 || input == 10 || input == 32 ) {
                QuexBuffer_input_p_increment(&me->buffer);
                goto TERMINAL_48_DIRECT;
            } else {
                goto STATE_236_DROP_OUT;
            }
        } else {
            if( input < 58) {
                if( input < 35) {
                        QuexBuffer_input_p_increment(&me->buffer);
                    goto TERMINAL_50_DIRECT;    /* '"' */
                } else {
                    if( input < 48) {
                        goto STATE_236_DROP_OUT_DIRECT;    /* ['#', '/'] */
                    } else {
                        goto STATE_246;    /* ['0', '9'] */
                    }
                }
            } else {
                if( input == 59) {
                    QuexBuffer_input_p_increment(&me->buffer);
                    goto TERMINAL_37_DIRECT;    /* ';' */
                } else {
                    goto STATE_236_DROP_OUT_DIRECT;    /* ':' */
                }
            }
        }
    } else {
        if( input < 115) {
            if( input < 97) {
                if( input == 61) {
                    QuexBuffer_input_p_increment(&me->buffer);
                    goto TERMINAL_28_DIRECT;    /* '=' */
                } else {
                    goto STATE_236_DROP_OUT_DIRECT;    /* ['>', '`'] */
                }
            } else {
                if( input == 105) {
                    goto STATE_239;    /* 'i' */
                } else {
                    goto STATE_245;    /* ['a', 'h'] */
                }
            }
        } else {
            if( input < 124) {
                if( input < 116) {
                        goto STATE_238;    /* 's' */
                } else {
                    if( input != 123) {
                        goto STATE_245;    /* ['t', 'z'] */
                    } else {
                        QuexBuffer_input_p_increment(&me->buffer);
                        goto TERMINAL_22_DIRECT;    /* '{' */
                    }
                }
            } else {
                if( input == 125) {
                    QuexBuffer_input_p_increment(&me->buffer);
                    goto TERMINAL_25_DIRECT;    /* '}' */
                } else {
                    goto STATE_236_DROP_OUT_DIRECT;    /* '|' */
                }
            }
        }
    }

STATE_236_DROP_OUT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_236_DROP_OUT");
    if( input != QUEX_SETTING_BUFFER_LIMIT_CODE ) {
STATE_236_DROP_OUT_DIRECT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_236_DROP_OUT_DIRECT");
        QUEX_GOTO_last_acceptance();

    }

    if( QuexBuffer_is_end_of_file(&me->buffer) ) {
        /* NO CHECK 'last_acceptance != -1' --- first state can **never** be an acceptance state */
        goto TERMINAL_END_OF_STREAM;
    }
        QUEX_DEBUG_PRINT(&me->buffer, "FORWARD_BUFFER_RELOAD");
    if( QuexAnalyser_buffer_reload_forward(&me->buffer, &last_acceptance_input_position,
                                           post_context_start_position, PostContextStartPositionN) ) {
       goto STATE_236_INPUT;
    }

    QUEX_DEBUG_PRINT(&me->buffer, "BUFFER_RELOAD_FAILED");
    QUEX_GOTO_last_acceptance();


STATE_236_INPUT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_236_INPUT");
    QuexBuffer_input_p_increment(&me->buffer);
    goto STATE_236;
STATE_238:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_238");

STATE_238_INPUT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_238_INPUT");

    QuexBuffer_input_p_increment(&me->buffer);
    input = QuexBuffer_input_get(&me->buffer);
    if( input < 116) {
        if( input < 97) {
            goto STATE_238_DROP_OUT;    /* [-oo, '`'] */
        } else {
            goto STATE_245;    /* ['a', 's'] */
        }
    } else {
        if( input < 117) {
                goto STATE_248;    /* 't' */
        } else {
            if( input < 123) {
                goto STATE_245;    /* ['u', 'z'] */
            } else {
                goto STATE_238_DROP_OUT_DIRECT;    /* ['{', oo] */
            }
        }
    }

STATE_238_DROP_OUT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_238_DROP_OUT");
    if( input != QUEX_SETTING_BUFFER_LIMIT_CODE ) {
STATE_238_DROP_OUT_DIRECT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_238_DROP_OUT_DIRECT");
            goto TERMINAL_44_DIRECT;
    }

        QUEX_DEBUG_PRINT2(&me->buffer, "ACCEPTANCE: %s", "44");
    QUEX_SET_last_acceptance(44);
    last_acceptance_input_position = QuexBuffer_tell_memory_adr(&me->buffer);
    
    QUEX_DEBUG_PRINT(&me->buffer, "FORWARD_BUFFER_RELOAD");
    if( QuexAnalyser_buffer_reload_forward(&me->buffer, &last_acceptance_input_position,
                                           post_context_start_position, PostContextStartPositionN) ) {
       goto STATE_238_INPUT;
    }

    QUEX_DEBUG_PRINT(&me->buffer, "BUFFER_RELOAD_FAILED");
    QUEX_GOTO_last_acceptance();



STATE_239:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_239");

STATE_239_INPUT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_239_INPUT");

    QuexBuffer_input_p_increment(&me->buffer);
    input = QuexBuffer_input_get(&me->buffer);
    if( input < 102) {
        if( input < 97) {
            goto STATE_239_DROP_OUT;    /* [-oo, '`'] */
        } else {
            goto STATE_245;    /* ['a', 'e'] */
        }
    } else {
        if( input < 103) {
                goto STATE_247;    /* 'f' */
        } else {
            if( input < 123) {
                goto STATE_245;    /* ['g', 'z'] */
            } else {
                goto STATE_239_DROP_OUT_DIRECT;    /* ['{', oo] */
            }
        }
    }

STATE_239_DROP_OUT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_239_DROP_OUT");
    if( input != QUEX_SETTING_BUFFER_LIMIT_CODE ) {
STATE_239_DROP_OUT_DIRECT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_239_DROP_OUT_DIRECT");
            goto TERMINAL_44_DIRECT;
    }

        QUEX_DEBUG_PRINT2(&me->buffer, "ACCEPTANCE: %s", "44");
    QUEX_SET_last_acceptance(44);
    last_acceptance_input_position = QuexBuffer_tell_memory_adr(&me->buffer);
    
    QUEX_DEBUG_PRINT(&me->buffer, "FORWARD_BUFFER_RELOAD");
    if( QuexAnalyser_buffer_reload_forward(&me->buffer, &last_acceptance_input_position,
                                           post_context_start_position, PostContextStartPositionN) ) {
       goto STATE_239_INPUT;
    }

    QUEX_DEBUG_PRINT(&me->buffer, "BUFFER_RELOAD_FAILED");
    QUEX_GOTO_last_acceptance();



STATE_245:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_245");

STATE_245_INPUT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_245_INPUT");

    QuexBuffer_input_p_increment(&me->buffer);
    input = QuexBuffer_input_get(&me->buffer);
    if( input >= 97 && input < 123 ) {
        goto STATE_245;    /* ['a', 'z'] */
    } else {
        goto STATE_245_DROP_OUT;    /* [-oo, '`'] */
    }

STATE_245_DROP_OUT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_245_DROP_OUT");
    if( input != QUEX_SETTING_BUFFER_LIMIT_CODE ) {
    /* STATE_245_DROP_OUT_DIRECT:
     */
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_245_DROP_OUT_DIRECT");
            goto TERMINAL_44_DIRECT;
    }

        QUEX_DEBUG_PRINT2(&me->buffer, "ACCEPTANCE: %s", "44");
    QUEX_SET_last_acceptance(44);
    last_acceptance_input_position = QuexBuffer_tell_memory_adr(&me->buffer);
    
    QUEX_DEBUG_PRINT(&me->buffer, "FORWARD_BUFFER_RELOAD");
    if( QuexAnalyser_buffer_reload_forward(&me->buffer, &last_acceptance_input_position,
                                           post_context_start_position, PostContextStartPositionN) ) {
       goto STATE_245_INPUT;
    }

    QUEX_DEBUG_PRINT(&me->buffer, "BUFFER_RELOAD_FAILED");
    QUEX_GOTO_last_acceptance();



STATE_246:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_246");

STATE_246_INPUT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_246_INPUT");

    QuexBuffer_input_p_increment(&me->buffer);
    input = QuexBuffer_input_get(&me->buffer);
    if( input >= 48 && input < 58 ) {
        goto STATE_246;    /* ['0', '9'] */
    } else {
        goto STATE_246_DROP_OUT;    /* [-oo, '/'] */
    }

STATE_246_DROP_OUT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_246_DROP_OUT");
    if( input != QUEX_SETTING_BUFFER_LIMIT_CODE ) {
    /* STATE_246_DROP_OUT_DIRECT:
     */
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_246_DROP_OUT_DIRECT");
            goto TERMINAL_45_DIRECT;
    }

        QUEX_DEBUG_PRINT2(&me->buffer, "ACCEPTANCE: %s", "45");
    QUEX_SET_last_acceptance(45);
    last_acceptance_input_position = QuexBuffer_tell_memory_adr(&me->buffer);
    
    QUEX_DEBUG_PRINT(&me->buffer, "FORWARD_BUFFER_RELOAD");
    if( QuexAnalyser_buffer_reload_forward(&me->buffer, &last_acceptance_input_position,
                                           post_context_start_position, PostContextStartPositionN) ) {
       goto STATE_246_INPUT;
    }

    QUEX_DEBUG_PRINT(&me->buffer, "BUFFER_RELOAD_FAILED");
    QUEX_GOTO_last_acceptance();



STATE_247:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_247");

STATE_247_INPUT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_247_INPUT");

    QuexBuffer_input_p_increment(&me->buffer);
    input = QuexBuffer_input_get(&me->buffer);
    if( input >= 97 && input < 123 ) {
        goto STATE_245;    /* ['a', 'z'] */
    } else {
        goto STATE_247_DROP_OUT;    /* [-oo, '`'] */
    }

STATE_247_DROP_OUT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_247_DROP_OUT");
    if( input != QUEX_SETTING_BUFFER_LIMIT_CODE ) {
    /* STATE_247_DROP_OUT_DIRECT:
     */
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_247_DROP_OUT_DIRECT");
            goto TERMINAL_34_DIRECT;
    }

        QUEX_DEBUG_PRINT2(&me->buffer, "ACCEPTANCE: %s", "34");
    QUEX_SET_last_acceptance(34);
    last_acceptance_input_position = QuexBuffer_tell_memory_adr(&me->buffer);
    
    QUEX_DEBUG_PRINT(&me->buffer, "FORWARD_BUFFER_RELOAD");
    if( QuexAnalyser_buffer_reload_forward(&me->buffer, &last_acceptance_input_position,
                                           post_context_start_position, PostContextStartPositionN) ) {
       goto STATE_247_INPUT;
    }

    QUEX_DEBUG_PRINT(&me->buffer, "BUFFER_RELOAD_FAILED");
    QUEX_GOTO_last_acceptance();



STATE_248:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_248");

STATE_248_INPUT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_248_INPUT");

    QuexBuffer_input_p_increment(&me->buffer);
    input = QuexBuffer_input_get(&me->buffer);
    if( input < 114) {
        if( input < 97) {
            goto STATE_248_DROP_OUT;    /* [-oo, '`'] */
        } else {
            goto STATE_245;    /* ['a', 'q'] */
        }
    } else {
        if( input < 115) {
                goto STATE_249;    /* 'r' */
        } else {
            if( input < 123) {
                goto STATE_245;    /* ['s', 'z'] */
            } else {
                goto STATE_248_DROP_OUT_DIRECT;    /* ['{', oo] */
            }
        }
    }

STATE_248_DROP_OUT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_248_DROP_OUT");
    if( input != QUEX_SETTING_BUFFER_LIMIT_CODE ) {
STATE_248_DROP_OUT_DIRECT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_248_DROP_OUT_DIRECT");
            goto TERMINAL_44_DIRECT;
    }

        QUEX_DEBUG_PRINT2(&me->buffer, "ACCEPTANCE: %s", "44");
    QUEX_SET_last_acceptance(44);
    last_acceptance_input_position = QuexBuffer_tell_memory_adr(&me->buffer);
    
    QUEX_DEBUG_PRINT(&me->buffer, "FORWARD_BUFFER_RELOAD");
    if( QuexAnalyser_buffer_reload_forward(&me->buffer, &last_acceptance_input_position,
                                           post_context_start_position, PostContextStartPositionN) ) {
       goto STATE_248_INPUT;
    }

    QUEX_DEBUG_PRINT(&me->buffer, "BUFFER_RELOAD_FAILED");
    QUEX_GOTO_last_acceptance();



STATE_249:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_249");

STATE_249_INPUT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_249_INPUT");

    QuexBuffer_input_p_increment(&me->buffer);
    input = QuexBuffer_input_get(&me->buffer);
    if( input < 117) {
        if( input < 97) {
            goto STATE_249_DROP_OUT;    /* [-oo, '`'] */
        } else {
            goto STATE_245;    /* ['a', 't'] */
        }
    } else {
        if( input < 118) {
                goto STATE_250;    /* 'u' */
        } else {
            if( input < 123) {
                goto STATE_245;    /* ['v', 'z'] */
            } else {
                goto STATE_249_DROP_OUT_DIRECT;    /* ['{', oo] */
            }
        }
    }

STATE_249_DROP_OUT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_249_DROP_OUT");
    if( input != QUEX_SETTING_BUFFER_LIMIT_CODE ) {
STATE_249_DROP_OUT_DIRECT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_249_DROP_OUT_DIRECT");
            goto TERMINAL_44_DIRECT;
    }

        QUEX_DEBUG_PRINT2(&me->buffer, "ACCEPTANCE: %s", "44");
    QUEX_SET_last_acceptance(44);
    last_acceptance_input_position = QuexBuffer_tell_memory_adr(&me->buffer);
    
    QUEX_DEBUG_PRINT(&me->buffer, "FORWARD_BUFFER_RELOAD");
    if( QuexAnalyser_buffer_reload_forward(&me->buffer, &last_acceptance_input_position,
                                           post_context_start_position, PostContextStartPositionN) ) {
       goto STATE_249_INPUT;
    }

    QUEX_DEBUG_PRINT(&me->buffer, "BUFFER_RELOAD_FAILED");
    QUEX_GOTO_last_acceptance();



STATE_250:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_250");

STATE_250_INPUT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_250_INPUT");

    QuexBuffer_input_p_increment(&me->buffer);
    input = QuexBuffer_input_get(&me->buffer);
    if( input < 99) {
        if( input == 97 || input == 98 ) {
            goto STATE_245;
        } else {
            goto STATE_250_DROP_OUT;
        }
    } else {
        if( input < 100) {
                goto STATE_251;    /* 'c' */
        } else {
            if( input < 123) {
                goto STATE_245;    /* ['d', 'z'] */
            } else {
                goto STATE_250_DROP_OUT_DIRECT;    /* ['{', oo] */
            }
        }
    }

STATE_250_DROP_OUT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_250_DROP_OUT");
    if( input != QUEX_SETTING_BUFFER_LIMIT_CODE ) {
STATE_250_DROP_OUT_DIRECT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_250_DROP_OUT_DIRECT");
            goto TERMINAL_44_DIRECT;
    }

        QUEX_DEBUG_PRINT2(&me->buffer, "ACCEPTANCE: %s", "44");
    QUEX_SET_last_acceptance(44);
    last_acceptance_input_position = QuexBuffer_tell_memory_adr(&me->buffer);
    
    QUEX_DEBUG_PRINT(&me->buffer, "FORWARD_BUFFER_RELOAD");
    if( QuexAnalyser_buffer_reload_forward(&me->buffer, &last_acceptance_input_position,
                                           post_context_start_position, PostContextStartPositionN) ) {
       goto STATE_250_INPUT;
    }

    QUEX_DEBUG_PRINT(&me->buffer, "BUFFER_RELOAD_FAILED");
    QUEX_GOTO_last_acceptance();



STATE_251:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_251");

STATE_251_INPUT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_251_INPUT");

    QuexBuffer_input_p_increment(&me->buffer);
    input = QuexBuffer_input_get(&me->buffer);
    if( input < 116) {
        if( input < 97) {
            goto STATE_251_DROP_OUT;    /* [-oo, '`'] */
        } else {
            goto STATE_245;    /* ['a', 's'] */
        }
    } else {
        if( input < 117) {
                goto STATE_252;    /* 't' */
        } else {
            if( input < 123) {
                goto STATE_245;    /* ['u', 'z'] */
            } else {
                goto STATE_251_DROP_OUT_DIRECT;    /* ['{', oo] */
            }
        }
    }

STATE_251_DROP_OUT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_251_DROP_OUT");
    if( input != QUEX_SETTING_BUFFER_LIMIT_CODE ) {
STATE_251_DROP_OUT_DIRECT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_251_DROP_OUT_DIRECT");
            goto TERMINAL_44_DIRECT;
    }

        QUEX_DEBUG_PRINT2(&me->buffer, "ACCEPTANCE: %s", "44");
    QUEX_SET_last_acceptance(44);
    last_acceptance_input_position = QuexBuffer_tell_memory_adr(&me->buffer);
    
    QUEX_DEBUG_PRINT(&me->buffer, "FORWARD_BUFFER_RELOAD");
    if( QuexAnalyser_buffer_reload_forward(&me->buffer, &last_acceptance_input_position,
                                           post_context_start_position, PostContextStartPositionN) ) {
       goto STATE_251_INPUT;
    }

    QUEX_DEBUG_PRINT(&me->buffer, "BUFFER_RELOAD_FAILED");
    QUEX_GOTO_last_acceptance();



STATE_252:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_252");

STATE_252_INPUT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_252_INPUT");

    QuexBuffer_input_p_increment(&me->buffer);
    input = QuexBuffer_input_get(&me->buffer);
    if( input >= 97 && input < 123 ) {
        goto STATE_245;    /* ['a', 'z'] */
    } else {
        goto STATE_252_DROP_OUT;    /* [-oo, '`'] */
    }

STATE_252_DROP_OUT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_252_DROP_OUT");
    if( input != QUEX_SETTING_BUFFER_LIMIT_CODE ) {
    /* STATE_252_DROP_OUT_DIRECT:
     */
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: STATE_252_DROP_OUT_DIRECT");
            goto TERMINAL_31_DIRECT;
    }

        QUEX_DEBUG_PRINT2(&me->buffer, "ACCEPTANCE: %s", "31");
    QUEX_SET_last_acceptance(31);
    last_acceptance_input_position = QuexBuffer_tell_memory_adr(&me->buffer);
    
    QUEX_DEBUG_PRINT(&me->buffer, "FORWARD_BUFFER_RELOAD");
    if( QuexAnalyser_buffer_reload_forward(&me->buffer, &last_acceptance_input_position,
                                           post_context_start_position, PostContextStartPositionN) ) {
       goto STATE_252_INPUT;
    }

    QUEX_DEBUG_PRINT(&me->buffer, "BUFFER_RELOAD_FAILED");
    QUEX_GOTO_last_acceptance();




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
TERMINAL_34:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: TERMINAL_34");

    QuexBuffer_seek_memory_adr(&me->buffer, last_acceptance_input_position);

TERMINAL_34_DIRECT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: TERMINAL_34_DIRECT");

    {
        
        #line 1 ""
        {
        self.counter.__shift_end_values_to_start_values();
        self.counter.count_NoNewline(2);
        
        #line 54 "simple.qx"
        #ifdef QUEX_OPTION_TOKEN_SENDING_VIA_QUEUE
        self.send(QUEX_TKN_IF); return;
        #else
        self.send(); return QUEX_TKN_IF;
        #endif
#line 1108 "tiny_lexer-core-engine.cpp"
        
        }
#line 1111 "tiny_lexer-core-engine.cpp"
        
    }

    goto __REENTRY_PREPARATION;

TERMINAL_37:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: TERMINAL_37");

    QuexBuffer_seek_memory_adr(&me->buffer, last_acceptance_input_position);

TERMINAL_37_DIRECT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: TERMINAL_37_DIRECT");

    {
        
        #line 1 ""
        {
        self.counter.__shift_end_values_to_start_values();
        self.counter.count_NoNewline(1);
        
        #line 55 "simple.qx"
        #ifdef QUEX_OPTION_TOKEN_SENDING_VIA_QUEUE
        self.send(QUEX_TKN_SEMICOLON); return;
        #else
        self.send(); return QUEX_TKN_SEMICOLON;
        #endif
#line 1138 "tiny_lexer-core-engine.cpp"
        
        }
#line 1141 "tiny_lexer-core-engine.cpp"
        
    }

    goto __REENTRY_PREPARATION;

TERMINAL_44:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: TERMINAL_44");

    QuexBuffer_seek_memory_adr(&me->buffer, last_acceptance_input_position);

TERMINAL_44_DIRECT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: TERMINAL_44_DIRECT");

    QuexBuffer_set_terminating_zero_for_lexeme(&me->buffer);
    {
        
        #line 1 ""
        {
        self.counter.__shift_end_values_to_start_values();
        self.counter.count_NoNewline(LexemeL);
        
        #line 57 "simple.qx"
        self.send(QUEX_TKN_IDENTIFIER, Lexeme); 
#line 1165 "tiny_lexer-core-engine.cpp"
        
        }
#line 1168 "tiny_lexer-core-engine.cpp"
        
    }

    goto __REENTRY_PREPARATION;

TERMINAL_45:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: TERMINAL_45");

    QuexBuffer_seek_memory_adr(&me->buffer, last_acceptance_input_position);

TERMINAL_45_DIRECT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: TERMINAL_45_DIRECT");

    QuexBuffer_set_terminating_zero_for_lexeme(&me->buffer);
    {
        
        #line 1 ""
        {
        self.counter.__shift_end_values_to_start_values();
        self.counter.count_NoNewline(LexemeL);
        
        #line 58 "simple.qx"
        #ifdef QUEX_OPTION_TOKEN_SENDING_VIA_QUEUE
        self.send(QUEX_TKN_NUMBER, atoi((const char*)Lexeme)); return;
        #else
        self.send(atoi((const char*)Lexeme)); return QUEX_TKN_NUMBER;
        #endif
#line 1196 "tiny_lexer-core-engine.cpp"
        
        }
#line 1199 "tiny_lexer-core-engine.cpp"
        
    }

    goto __REENTRY_PREPARATION;

TERMINAL_48:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: TERMINAL_48");

    QuexBuffer_seek_memory_adr(&me->buffer, last_acceptance_input_position);

TERMINAL_48_DIRECT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: TERMINAL_48_DIRECT");

    QuexBuffer_set_terminating_zero_for_lexeme(&me->buffer);
    {
        
        #line 1 ""
        {
        self.counter.__shift_end_values_to_start_values();
        self.counter.count(Lexeme, LexemeEnd);
        
        }
#line 1222 "tiny_lexer-core-engine.cpp"
        
    }

    goto __REENTRY_PREPARATION;

TERMINAL_50:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: TERMINAL_50");

    QuexBuffer_seek_memory_adr(&me->buffer, last_acceptance_input_position);

TERMINAL_50_DIRECT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: TERMINAL_50_DIRECT");

    {
        
        #line 1 ""
        {
        self.counter.__shift_end_values_to_start_values();
        self.counter.count_NoNewline(1);
        
        #line 63 "simple.qx"
        self.send(QUEX_TKN_QUOTE);
                self << STRING_READER;
                RETURN;
            
#line 1248 "tiny_lexer-core-engine.cpp"
        
        }
#line 1251 "tiny_lexer-core-engine.cpp"
        
    }

    goto __REENTRY_PREPARATION;

TERMINAL_22:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: TERMINAL_22");

    QuexBuffer_seek_memory_adr(&me->buffer, last_acceptance_input_position);

TERMINAL_22_DIRECT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: TERMINAL_22_DIRECT");

    {
        
        #line 1 ""
        {
        self.counter.__shift_end_values_to_start_values();
        self.counter.count_NoNewline(1);
        
        #line 50 "simple.qx"
        self.send(QUEX_TKN_CURLY_BRACKET_O); RETURN; 
#line 1274 "tiny_lexer-core-engine.cpp"
        
        }
#line 1277 "tiny_lexer-core-engine.cpp"
        
    }

    goto __REENTRY_PREPARATION;

TERMINAL_25:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: TERMINAL_25");

    QuexBuffer_seek_memory_adr(&me->buffer, last_acceptance_input_position);

TERMINAL_25_DIRECT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: TERMINAL_25_DIRECT");

    {
        
        #line 1 ""
        {
        self.counter.__shift_end_values_to_start_values();
        self.counter.count_NoNewline(1);
        
        #line 51 "simple.qx"
        #ifdef QUEX_OPTION_TOKEN_SENDING_VIA_QUEUE
        self.send(QUEX_TKN_CURLY_BRACKET_C); return;
        #else
        self.send(); return QUEX_TKN_CURLY_BRACKET_C;
        #endif
#line 1304 "tiny_lexer-core-engine.cpp"
        
        }
#line 1307 "tiny_lexer-core-engine.cpp"
        
    }

    goto __REENTRY_PREPARATION;

TERMINAL_28:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: TERMINAL_28");

    QuexBuffer_seek_memory_adr(&me->buffer, last_acceptance_input_position);

TERMINAL_28_DIRECT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: TERMINAL_28_DIRECT");

    {
        
        #line 1 ""
        {
        self.counter.__shift_end_values_to_start_values();
        self.counter.count_NoNewline(1);
        
        #line 52 "simple.qx"
        #ifdef QUEX_OPTION_TOKEN_SENDING_VIA_QUEUE
        self.send(QUEX_TKN_OP_ASSIGNMENT); return;
        #else
        self.send(); return QUEX_TKN_OP_ASSIGNMENT;
        #endif
#line 1334 "tiny_lexer-core-engine.cpp"
        
        }
#line 1337 "tiny_lexer-core-engine.cpp"
        
    }

    goto __REENTRY_PREPARATION;

TERMINAL_31:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: TERMINAL_31");

    QuexBuffer_seek_memory_adr(&me->buffer, last_acceptance_input_position);

TERMINAL_31_DIRECT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: TERMINAL_31_DIRECT");

    {
        
        #line 1 ""
        {
        self.counter.__shift_end_values_to_start_values();
        self.counter.count_NoNewline(6);
        
        #line 53 "simple.qx"
        #ifdef QUEX_OPTION_TOKEN_SENDING_VIA_QUEUE
        self.send(QUEX_TKN_STRUCT); return;
        #else
        self.send(); return QUEX_TKN_STRUCT;
        #endif
#line 1364 "tiny_lexer-core-engine.cpp"
        
        }
#line 1367 "tiny_lexer-core-engine.cpp"
        
    }

    goto __REENTRY_PREPARATION;



TERMINAL_END_OF_STREAM:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: TERMINAL_END_OF_STREAM");

                {
                    
        #line 1 ""
        {
        self.counter.__shift_end_values_to_start_values();
        
        }
#line 1385 "tiny_lexer-core-engine.cpp"
        
                }

#ifdef __QUEX_OPTION_ANALYSER_RETURN_TYPE_IS_VOID
        return /*__QUEX_TOKEN_ID_TERMINATION*/;
#else
        return __QUEX_TOKEN_ID_TERMINATION;
#endif

TERMINAL_DEFAULT:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: TERMINAL_DEFAULT");

me->buffer._input_p = me->buffer._lexeme_start_p;
if( QuexBuffer_is_end_of_file(&me->buffer) ) {

    /* Next increment will stop on EOF character. */
}

else {
    /* Step over nomatching character */    QuexBuffer_input_p_increment(&me->buffer);
}

                QuexBuffer_set_terminating_zero_for_lexeme(&me->buffer);
                {
                    
        #line 1 ""
        {
        self.counter.__shift_end_values_to_start_values();
        self.counter.count(Lexeme, LexemeEnd);
        
        }
#line 1417 "tiny_lexer-core-engine.cpp"
        
                }

        goto __REENTRY_PREPARATION;

#undef Lexeme
#undef LexemeBegin
#undef LexemeEnd
#undef LexemeL
#ifndef __QUEX_OPTION_USE_COMPUTED_GOTOS
__TERMINAL_ROUTER: {
        /*  if last_acceptance => goto correspondent acceptance terminal state*/
        /*  else               => execute defaul action*/
        switch( last_acceptance ) {
            case 34: goto TERMINAL_34;
            case 37: goto TERMINAL_37;
            case 44: goto TERMINAL_44;
            case 45: goto TERMINAL_45;
            case 48: goto TERMINAL_48;
            case 50: goto TERMINAL_50;
            case 22: goto TERMINAL_22;
            case 25: goto TERMINAL_25;
            case 28: goto TERMINAL_28;
            case 31: goto TERMINAL_31;

            default: goto TERMINAL_DEFAULT;; /* nothing matched */
        }
    }
#endif /* __QUEX_OPTION_USE_COMPUTED_GOTOS */

  
__REENTRY_PREPARATION:
    QUEX_DEBUG_PRINT(&me->buffer, "LABEL: __REENTRY_PREPARATION");

    /* (*) Common point for **restarting** lexical analysis.
     *     at each time when CONTINUE is called at the end of a pattern. */
    last_acceptance = QUEX_GOTO_TERMINAL_LABEL_INIT_VALUE;


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
#if    defined(QUEX_OPTION_AUTOMATIC_ANALYSIS_CONTINUATION_ON_MODE_CHANGE)     || defined(QUEX_OPTION_ASSERTS)
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

    goto __REENTRY;

    /* prevent compiler warning 'unused variable': use variables once in a part of the code*/
    /* that is never reached (and deleted by the compiler anyway).*/
    if( 0 == 1 ) {
        int unused = 0;
        unused = unused + STRING_READER.id;
        unused = unused + PROGRAM.id;
    }
}
#include <quex/code_base/temporary_macros_off>
#if ! defined(__QUEX_SETTING_PLAIN_C)
} // namespace quex
#endif
