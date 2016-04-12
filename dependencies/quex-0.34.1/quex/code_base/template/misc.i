// -*- C++ -*-   vim: set syntax=cpp:

namespace quex { 
    inline void    
    CLASS::move_forward(const size_t CharacterN)
    {
        QuexBuffer_move_forward(&this->buffer, CharacterN);
    }

    inline void    
    CLASS::move_backward(const size_t CharacterN)
    {
        QuexBuffer_move_backward(&this->buffer, CharacterN);
    }

    
    inline size_t  
    CLASS::tell()
    {
        return QuexBuffer_tell(&this->buffer);
    }

    inline void    
    CLASS::seek(const size_t CharacterIndex)
    {
        QuexBuffer_seek(&this->buffer, CharacterIndex);
    }

    inline void
    CLASS::_reset()
    {
        QuexAnalyser_reset((QuexAnalyser*)this);

#   if   defined(QUEX_OPTION_LINE_NUMBER_COUNTING)          \
           | defined(QUEX_OPTION_COLUMN_NUMBER_COUNTING)        \
           | defined(__QUEX_OPTION_INDENTATION_TRIGGER_SUPPORT)
        counter.init();
#   endif

        // empty the token queue
#   ifdef QUEX_OPTION_TOKEN_SENDING_VIA_QUEUE
        this->_token_queue->reset();
#   endif

        set_mode_brutally(__QUEX_SETTING_INITIAL_LEXER_MODE_ID);
    }

} // namespace quex
