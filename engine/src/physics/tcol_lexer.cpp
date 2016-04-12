#include"tcol_lexer"
namespace quex {
        QuexMode  tcol_lexer::COMMENT;
        QuexMode  tcol_lexer::END_OF_FILE;
        QuexMode  tcol_lexer::MAIN;
#define self  (*me)

    void
    tcol_lexer_COMMENT_on_entry(tcol_lexer* me, const QuexMode* FromMode) {
#ifdef __QUEX_OPTION_RUNTIME_MODE_TRANSITION_CHECK
__quex_assert(me->COMMENT.has_entry_from(FromMode));
#endif

    }

    void
    tcol_lexer_COMMENT_on_exit(tcol_lexer* me, const QuexMode* ToMode)  {
#ifdef __QUEX_OPTION_RUNTIME_MODE_TRANSITION_CHECK
__quex_assert(me->COMMENT.has_exit_to(ToMode));
#endif

    }

#ifdef __QUEX_OPTION_INDENTATION_TRIGGER_SUPPORT        
    void
    tcol_lexer_COMMENT_on_indentation(tcol_lexer* me, const int Indentation) {
__quex_assert(Indentation >= 0);
    }
#endif

#ifdef __QUEX_OPTION_RUNTIME_MODE_TRANSITION_CHECK
    bool
    tcol_lexer_COMMENT_has_base(const QuexMode* Mode) {

    switch( Mode->id ) {
    case LEX_ID_END_OF_FILE: return true;
    default:
    ;
    }
    #ifdef __QUEX_OPTION_ERROR_OUTPUT_ON_MODE_CHANGE_ERROR
    std::cerr << "mode 'END_OF_FILE' is not one of:\n";    std::cerr << "         END_OF_FILE\n";
    #endif // QUEX_OPTION_ERROR_OUTPUT_ON_MODE_CHANGE_ERROR
    return false;
    
    }
    bool
    tcol_lexer_COMMENT_has_entry_from(const QuexMode* Mode) {
    return true; // default
    }
    bool
    tcol_lexer_COMMENT_has_exit_to(const QuexMode* Mode) {
    return true; // default
    }
#endif    

    void
    tcol_lexer_END_OF_FILE_on_entry(tcol_lexer* me, const QuexMode* FromMode) {
#ifdef __QUEX_OPTION_RUNTIME_MODE_TRANSITION_CHECK
__quex_assert(me->END_OF_FILE.has_entry_from(FromMode));
#endif

    }

    void
    tcol_lexer_END_OF_FILE_on_exit(tcol_lexer* me, const QuexMode* ToMode)  {
#ifdef __QUEX_OPTION_RUNTIME_MODE_TRANSITION_CHECK
__quex_assert(me->END_OF_FILE.has_exit_to(ToMode));
#endif

    }

#ifdef __QUEX_OPTION_INDENTATION_TRIGGER_SUPPORT        
    void
    tcol_lexer_END_OF_FILE_on_indentation(tcol_lexer* me, const int Indentation) {
__quex_assert(Indentation >= 0);
    }
#endif

#ifdef __QUEX_OPTION_RUNTIME_MODE_TRANSITION_CHECK
    bool
    tcol_lexer_END_OF_FILE_has_base(const QuexMode* Mode) {
    return false;
    }
    bool
    tcol_lexer_END_OF_FILE_has_entry_from(const QuexMode* Mode) {
    return true; // default
    }
    bool
    tcol_lexer_END_OF_FILE_has_exit_to(const QuexMode* Mode) {
    return true; // default
    }
#endif    

    void
    tcol_lexer_MAIN_on_entry(tcol_lexer* me, const QuexMode* FromMode) {
#ifdef __QUEX_OPTION_RUNTIME_MODE_TRANSITION_CHECK
__quex_assert(me->MAIN.has_entry_from(FromMode));
#endif

    }

    void
    tcol_lexer_MAIN_on_exit(tcol_lexer* me, const QuexMode* ToMode)  {
#ifdef __QUEX_OPTION_RUNTIME_MODE_TRANSITION_CHECK
__quex_assert(me->MAIN.has_exit_to(ToMode));
#endif

    }

#ifdef __QUEX_OPTION_INDENTATION_TRIGGER_SUPPORT        
    void
    tcol_lexer_MAIN_on_indentation(tcol_lexer* me, const int Indentation) {
__quex_assert(Indentation >= 0);
    }
#endif

#ifdef __QUEX_OPTION_RUNTIME_MODE_TRANSITION_CHECK
    bool
    tcol_lexer_MAIN_has_base(const QuexMode* Mode) {

    switch( Mode->id ) {
    case LEX_ID_END_OF_FILE: return true;
    default:
    ;
    }
    #ifdef __QUEX_OPTION_ERROR_OUTPUT_ON_MODE_CHANGE_ERROR
    std::cerr << "mode 'END_OF_FILE' is not one of:\n";    std::cerr << "         END_OF_FILE\n";
    #endif // QUEX_OPTION_ERROR_OUTPUT_ON_MODE_CHANGE_ERROR
    return false;
    
    }
    bool
    tcol_lexer_MAIN_has_entry_from(const QuexMode* Mode) {
    return true; // default
    }
    bool
    tcol_lexer_MAIN_has_exit_to(const QuexMode* Mode) {
    return true; // default
    }
#endif    
#undef self
} // END: namespace quex
