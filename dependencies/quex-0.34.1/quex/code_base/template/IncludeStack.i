// -*- C++ -*- vim:set syntax=cpp:
#ifndef __INCLUDE_GUARD__QUEX__INCLUDE_STACK
#define __INCLUDE_GUARD__QUEX__INCLUDE_STACK

#include <quex/code_base/template/Analyser>
namespace quex { 

    inline 
    IncludeStack::IncludeStack(CLASS* the_lexer)
        : _the_lexer(the_lexer)
    { }


    template <class InputHandle> inline void    
    IncludeStack::push(InputHandle*             new_input_handle_p, 
                       const QuexMode&          mode, 
                       QuexBufferFillerTypeEnum BFT /* = QUEX_AUTO */,
                       const char*              IANA_CodingName /* = 0x0 */)
    {
        // Once we allow MODE_ID == 0, reset the range to [0:MAX_MODE_CLASS_N]
        __push(new_input_handle_p, mode.analyser_function, IANA_CodingName);
    }

    template <class InputHandle> inline void    
    IncludeStack::push(InputHandle*             new_input_handle_p, 
                       const int                MODE_ID /* = -1 */, 
                       QuexBufferFillerTypeEnum BFT /* QUEX_AUTO */,
                       const char*              IANA_CodingName /* = 0x0 */)
    {
        // Once we allow MODE_ID == 0, reset the range to [0:MAX_MODE_CLASS_N]
        __quex_assert(    MODE_ID == -1 
                      || (MODE_ID >= 1 && MODE_ID < __QUEX_SETTING_MAX_MODE_CLASS_N + 1));
        __quex_assert(new_input_handle_p != 0x0);
        // IANA_CodingName == 0x0 possible if normal ASCII is ment (e.g. no iconv support)

        if( MODE_ID != -1 ) _the_lexer->set_mode_brutally(MODE_ID);

        _stack.push_back(memento());
        memento*  m = &(_stack.back());

        // (1) saving the current state of the lexical analyzer (memento pattern)
        m->map_from_lexical_analyzer(_the_lexer);

        _the_lexer->counter.init();

        // (2) initializing the new state of the lexer for reading the new input file/stream
        QuexAnalyser_construct((QuexAnalyser*)_the_lexer,
                               _the_lexer->current_analyser_function,
                               new_input_handle_p,
                               BFT, IANA_CodingName, 
                               QUEX_SETTING_BUFFER_SIZE,
                               QUEX_SETTING_TRANSLATION_BUFFER_SIZE);
    }   

    inline bool
    IncludeStack::pop() 
    {
        /* Not included? return 'false' to indicate we're on the top level */
        if( _stack.empty() ) return false; 

        memento*  m = &(_stack.back());

        // (1) Free the related memory that is no longer used
        QuexAnalyser_destruct((QuexAnalyser*)_the_lexer);

        // (2) Reset the lexical analyser to the state it was before the include
        m->map_to_lexical_analyzer(_the_lexer);

        // (3) Forget about the memento
        _stack.pop_back();

        /* Return to including file succesful */
        return true;
    }

    inline void
    IncludeStack::memento::map_from_lexical_analyzer(CLASS* TheLexer)
    {
        // (1) saving the current state of the lexical analyzer (memento pattern)
        this->analyser_core = *((QuexAnalyser*)TheLexer);
        this->counter       = TheLexer->counter;

        this->current_mode_p = TheLexer->__current_mode_p;
    }

    inline void
    IncludeStack::memento::map_to_lexical_analyzer(CLASS* the_lexer)
    {
        // (1) saving the current state of the lexical analyzer (memento pattern)
        *((QuexAnalyser*)the_lexer) = this->analyser_core;
        the_lexer->counter          = this->counter;
        the_lexer->__current_mode_p = this->current_mode_p;
    }
} // namespace quex

#endif // __INCLUDE_GUARD__QUEX__INCLUDE_STACK
