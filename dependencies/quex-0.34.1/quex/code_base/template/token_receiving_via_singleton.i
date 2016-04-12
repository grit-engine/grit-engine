// -*- C++ -*-   vim: set syntax=cpp:

namespace quex { 
    inline QUEX_TOKEN_ID_TYPE
    CLASS::get_token() 
    {
        // The framework / constructor **should** ensure that at this point the two
        // pointers are identical. Since this function is called very often the
        // assignment of safety (prev=current) is not done. Instead, we only check
        // (as long as NDEBUG is not defined) that the framework assigns the variables
        // propperly.
#       if ! defined(QUEX_OPTION_AUTOMATIC_ANALYSIS_CONTINUATION_ON_MODE_CHANGE)
        return QuexAnalyser::current_analyser_function(this);
#       else
        QUEX_TOKEN_ID_TYPE token_id = __QUEX_TOKEN_ID_TERMINATION;
        do {
            token_id = QuexAnalyser::current_analyser_function(this);
        } while( token_id == __QUEX_TOKEN_ID_UNINITIALIZED );
        return token_id;
#       endif
    }
}
