// -*- C++ -*-   vim: set syntax=cpp:
namespace quex { 
    inline void
    CLASS::get_token(__QUEX_SETTING_TOKEN_CLASS_NAME** result_pp) 
    // NOTE: As long as the 'get_token()' function is not called there is nothing
    //       happening to the token in the queue. But, a parser very probably
    //       does a couple af calls to 'get_token()' before a rule triggers 
    //       and data structures can be stored.
    //
    // ARGUMENTS:
    //     result_p  points to memory where token information has to be stored.
    // TIP:
    //     result_p could point into the token queue directly (TODO), if a limit 
    //     number can be defined so that the token queue does not overwrite it, as
    //     long as the parser is chewing on it.
    //
    // RETURNS:
    //    Token-ID of the currently read token.
    //    Token-ID = '$$TOKEN_CLASS$$::ID_UNITIALIZED' is returned in 
    //               case that no  token could be read.
    {
        __quex_assert(_token_queue != 0x0);
        // (i) tokens are in queue --> take next token from stack
        if( _token_queue->is_empty() == false ) {
            // DEBUG    
            *result_pp = _token_queue->quick_pop();
            return;
        }

#       if ! defined(QUEX_OPTION_AUTOMATIC_ANALYSIS_CONTINUATION_ON_MODE_CHANGE)
        QuexAnalyser::current_analyser_function(this);
#       else
        do { 
            QuexAnalyser::current_analyser_function(this);

            // In case a mode change happend inside the pattern actions, the function is forced
            // to return (see end of analyzer function at REENTRY label). If the tokenstack is
            // non-empty, we return to the caller (spare one check). If its empty the analyzer
            // function (which has recently been setup) is called again.
        } while ( _token_queue->is_empty() );
#       endif

        *result_pp = _token_queue->quick_pop();
        return;
    }

    inline QUEX_TOKEN_ID_TYPE
    CLASS::get_token(__QUEX_SETTING_TOKEN_CLASS_NAME* result_p) 
    {
        __QUEX_SETTING_TOKEN_CLASS_NAME* tmp = 0x0;
        get_token(&tmp);
        *result_p = *tmp;
        return result_p->type_id();
    }


}
