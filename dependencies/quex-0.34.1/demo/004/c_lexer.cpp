#include"c_lexer"
namespace quex {
        QuexMode  c_lexer::PROGRAM;
#define self  (*me)

    void
    c_lexer_PROGRAM_on_entry(c_lexer* me, const QuexMode* FromMode) {
#ifdef __QUEX_OPTION_RUNTIME_MODE_TRANSITION_CHECK
__quex_assert(me->PROGRAM.has_entry_from(FromMode));
#endif

    }

    void
    c_lexer_PROGRAM_on_exit(c_lexer* me, const QuexMode* ToMode)  {
#ifdef __QUEX_OPTION_RUNTIME_MODE_TRANSITION_CHECK
__quex_assert(me->PROGRAM.has_exit_to(ToMode));
#endif

    }

#ifdef __QUEX_OPTION_INDENTATION_TRIGGER_SUPPORT        
    void
    c_lexer_PROGRAM_on_indentation(c_lexer* me, const int Indentation) {
__quex_assert(Indentation >= 0);
    }
#endif

#ifdef __QUEX_OPTION_RUNTIME_MODE_TRANSITION_CHECK
    bool
    c_lexer_PROGRAM_has_base(const QuexMode* Mode) {
    return false;
    }
    bool
    c_lexer_PROGRAM_has_entry_from(const QuexMode* Mode) {
    return true; // default
    }
    bool
    c_lexer_PROGRAM_has_exit_to(const QuexMode* Mode) {
    return true; // default
    }
#endif    
#undef self
} // END: namespace quex
