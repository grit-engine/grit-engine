/*
    // (*) utilities for string triming
    //
    //     NOTE: Any better linker should be able to inline the 
    //           condition functions, so do not worry about the 
    //           function pointers.
    inline void
    trim_rear(QUEX_LEXEME_CHARACTER_TYPE*     text, 
              const int Length,
              int       (*condition)(const QUEX_CHARACTER_TYPE))
        // PURPOSE:
        //    Inserts a terminating zero after the first letter where a
        //    condition hold. This is useful, for example, if strings have 
        //    to be created and whitespace at borders shall not take space.
        //
        //    The length of the text may already been determined. Pass Length,
        //    so no call to strlen is necessary here.
        //
        // text       = text to be parsed for condition
        // Length     = Length of the string
        // condition  = pointer to function that checks for condition
        //              return 1 = condition holds
        //              return 0 = condition does not hold
        //              (examples: the functions isspace(..), islower(..)
        //                         from standard <ctype.h> header)
    {
        QUEX_LEXEME_CHARACTER_TYPE* p = text + Length - 1;
        for(; p > text && (*condition)(*p); --p);
        *(++p) = '\0'; // terminating zero
    }

    inline const QUEX_LEXEME_CHARACTER_TYPE*
    trim_front(QUEX_LEXEME_CHARACTER_TYPE* text, 
               int   (*condition)(const QUEX_CHARACTER_TYPE))
        // PURPOSE:
        //    Like insert_term_zero_after_condition(...) treating the string
        //    from front to back (i.e. vice versa). Does not insert terminating
        //    zero, but returns the pointer to the first non-whitespace.
        //
        // RETURNS:
        //    pointer to first character that is not whitespace.
        //
    {       
        QUEX_LEXEME_CHARACTER_TYPE* p = text; 
        for(; *p && (*condition)(*p); ++p);
        return p;
    }

    inline const QUEX_LEXEME_CHARACTER_TYPE*
    trim(QUEX_LEXEME_CHARACTER_TYPE*     text, 
         const int Length,
         int       (*condition)(const QUEX_CHARACTER_TYPE))
        // PURPOSE:
        //    Delivers a string that has no whitespace at front and none at end.
        //    For example, the string "  hello world!    " becomes "hello world!".
        //
        //    The length of the text may already been determined, so no call
        //    to strlen is necessary here.
        //
        // ARGUMENTS:
        //    text      = text to be trimmed. 
        //    Length    = length of the original string.
        //    condition = condition function
        //
        // ATTENTION: The same memory of text is used for the return value. 
        //            But, it is modified! The content of 'text' may not
        //            be the same after this function call, since a terminating
        //            zero is forced after the last non-whitespace character.
        //
        // RETURNS:
        //    Pointer to the first non-whitespace character of text. At the
        //    end of the last non-whitespace character, there is going to be
        //    a '\0' determining the string border.
        //
    { trim_rear(text, Length, condition); return trim_front(text, condition); }

    inline void
    trim_rear(QUEX_LEXEME_CHARACTER_TYPE* text, const int Length)
    { trim_rear(text, Length, &isspace); }

    inline const QUEX_LEXEME_CHARACTER_TYPE*
    trim_front(QUEX_LEXEME_CHARACTER_TYPE* text)
    { trim_front(text, &isspace); }

    inline const QUEX_LEXEME_CHARACTER_TYPE*
    trim(QUEX_LEXEME_CHARACTER_TYPE* text, const int Length)
    { return trim(text, Length, &isspace); }
*/

