// -*- C++ -*- vim:set syntax=cpp:

#ifdef QUEX_OPTION_DEBUG_TOKEN_SENDING
#   define __QUEX_DEBUG_TOKEN_SENDING() \
        std::cerr << "$$LEXER_CLASS_NAME$$::send " << _token << std::endl;
#else
#   define __QUEX_DEBUG_TOKEN_SENDING() /* nothing */
#endif

namespace quex { 
    inline void   
    CLASS::send() 
    { 
        /* This function exists only, so that code generation looks uniform. 
         * It is empty, so it does not harm at all-it's simply optimized away. */
    }

    inline void   
    CLASS::send(const __QUEX_SETTING_TOKEN_CLASS_NAME& That) 
    {
        the_token = That;
        __QUEX_DEBUG_TOKEN_SENDING();
    }

    inline void   
    CLASS::send(const QUEX_TOKEN_ID_TYPE ID) 
    {
        the_token.set(ID);
        __QUEX_DEBUG_TOKEN_SENDING();
    }

    inline void   
    CLASS::send_n(const int N, QUEX_TOKEN_ID_TYPE ID) 
    {
        /* this function does not make sense for singleton tokens */
        the_token.set(ID); // applies DEBUG of 'send()'
        __QUEX_DEBUG_TOKEN_SENDING();
    }

    template <typename ContentT> inline void   
    CLASS::send(const QUEX_TOKEN_ID_TYPE ID, ContentT Content) 
    {
        the_token.set(ID, Content);
        __QUEX_DEBUG_TOKEN_SENDING();
    }
}
#undef __QUEX_DEBUG_TOKEN_SENDING


