// -*- C++ -*-   vim: set syntax=cpp:

#ifdef __QUEX_OPTION_INDENTATION_TRIGGER_SUPPORT	
#include <quex/code_base/template/count_common>

namespace quex { 
// NOTE: Quex is pretty intelligent in choosing the right function
//       to count line and column numbers. If, for example, a pattern
//       does not contain newlines, then it simply adds the LexemeLength
//       to the column number and does not do anything to the line number.
//       Before touching anything in this code, first look at the generated
//       code. The author of these lines considers it rather difficult to
//       find better implementations of these functions in the framework
//       of the generated engine.  <fschaef 07y6m30d>
//
// NOTE: Those functions are not responsible for setting the begin to the
//       last end, such as _line_number_at_begin = _line_number_at_end.
//       This has to happen outside these functions.
inline 
CounterWithIndentation::CounterWithIndentation(CLASS* TheLexer)
    : _the_lexer(TheLexer)
{ init(); }

inline 
CounterWithIndentation::CounterWithIndentation(const CounterWithIndentation& That)
    : _the_lexer(That._the_lexer)
{ 

    _indentation                 = That._indentation;
    _indentation_count_enabled_f = That._indentation_count_enabled_f;
    _indentation_event_enabled_f = That._indentation_event_enabled_f;
}

inline void
CounterWithIndentation::init()
{
    _indentation = 0;
    _indentation_count_enabled_f = false;
    _indentation_event_enabled_f = true;
}

inline void
CounterWithIndentation::on_end_of_file()
{
    // 'flush' remaining indentations
    if( _indentation_event_enabled_f ) 
        _the_lexer->mode().on_indentation(_the_lexer, _indentation);
}

inline void    
CounterWithIndentation::icount(QUEX_CHARACTER_TYPE* Lexeme,
                               QUEX_CHARACTER_TYPE* LexemeEnd)
// Lexeme:    Pointer to first character of Lexeme.
// LexemeEnd: Pointer to first character after Lexeme.
//
// PURPOSE:
//   Adapts the column number and the line number according to the newlines
//   and letters of the last line occuring in the lexeme.
//
////////////////////////////////////////////////////////////////////////////////
{
    //  NOTE: Indentation counting is only active between newline and the
    //        first non-whitespace character in a line. Use a variable
    //        to indicate the activation of newline.
    //  
    //  DEF: End_it   = pointer to first letter behind lexeme
    //       Begin_it = pointer to first letter of lexeme
    //  
    //  (1) last character of Lexeme == newline?
    //      yes => indentation_counting = ON
    //             indentation   = 0
    //             line_number_at_end   += number of newlines in Lexeme
    //             column_number_at_end  = 0           
    //      END
    //  
    //  (2) find last newline in Lexeme -> start_consideration_it
    //  
    //      (2.1) no newline in lexeme?
    //      yes => indentation_counting == ON ?
    //             no => perform normal line number and column number counting
    //                   (we are not in between newline and the first non-whitespace 
    //                    character of a line).
    //                   END
    //             yes => start_consideration_it = Begin_it
    //  
    //  (3) Count
    //  
    //      indentation  += number of whitespace between start_consideration 
    //                      and first non-whitespace character
    //      did non-whitespace character arrive?
    //         yes => indentation_counting = OFF
    //  
    //      column_number_at_end  = End_it - start_consideration_it
    //      line_number_at_end   += number of newlines from: Begin_it to: start_consideration_it
    //  
    QUEX_CHARACTER_TYPE* Begin = (QUEX_CHARACTER_TYPE*)Lexeme;
    QUEX_CHARACTER_TYPE* Last  = LexemeEnd - 1;                
    QUEX_CHARACTER_TYPE* it    = Last;

    __quex_assert(Begin < LexemeEnd);   // LexemeLength >= 1: NEVER COMPROMISE THIS !


    // (1) Last character == newline ? _______________________________________________
    //
    if( *Last == '\n' ) {
        _indentation = 0;
        _indentation_count_enabled_f = true;
#       ifndef  QUEX_NO_SUPPORT_FOR_LINE_NUMBER_COUNTING
        ++_line_number_at_end;
        __count_newline_n_backwards(it, Begin);
#       endif
#       ifndef  QUEX_NO_SUPPORT_FOR_COLUMN_NUMBER_COUNTING
        _column_number_at_end = 1;            // next lexeme starts at _column_number_at_end + 1
#       endif
        return;
    }

    // (2) Find last newline in lexeme _______________________________________________
    //
    QUEX_CHARACTER_TYPE* start_consideration_it = 0x0;
    it = Last;
    while( it != Begin ) {
        // recall assert: no lexeme with len(Lexeme) == 0
        --it;
        if( *it == '\n' ) { 		
            // NOTE: according to the test in (1) it is not possible
            //       that *it == "\n" and it == Last 
            //       => there is always an iterator behind 'it'
            //          if *it == "\n". The incrementation does
            //          not need a check;
            start_consideration_it = it;
            ++start_consideration_it;  // point to first character after newline
            _indentation = 0;
            _indentation_count_enabled_f = true;
#           ifdef QUEX_OPTION_COLUMN_NUMBER_COUNTING
            _column_number_at_end = 1;
#           endif
            break; 
        }	    
    }
    // (2.1) no newline in lexeme?
    if( start_consideration_it == 0x0 ) {
        if( _indentation_count_enabled_f == false ) {
            // count increment without consideration of indentations
            // no newline => no line number increment
            //               no column number overflow / restart at '1'
            // no indentation enabled => no indentation increment
#           ifdef QUEX_OPTION_COLUMN_NUMBER_COUNTING
            _column_number_at_end += LexemeEnd - Begin;
#           endif
            return;
        }
        // There was no newline, but the flag '_indentation_count_enabled_f'
        // tells us that before this pattern there was only whitespace after 
        // newline. Let us add the whitespace at the beginning of this pattern
        // to the _indentation.
        start_consideration_it = Begin;
    }
    // At this point:
    //   -- either there was no newline in the pattern, but the indentation
    //      count was active (see case above).
    //   -- or there was a newline in the pattern, so above it is set
    //      '_indentation_count_enabled_f = true'.
    __quex_assert( _indentation_count_enabled_f == true );

    // (3) Count _____________________________________________________________________
    //
    // -- whitespace from: start_consideration to first non-whitespace
    //    (indentation count is disabled if non-whitespace arrives)
    __count_whitespace_to_first_non_whitespace(start_consideration_it, Begin, LexemeEnd, 
                                               /* LicenseToIncrementLineCountF = */ true);

    __QUEX_LEXER_COUNT_ASSERT_CONSISTENCY();
}


inline void    
CounterWithIndentation::icount_NoNewline(QUEX_CHARACTER_TYPE* Lexeme,
                                         const int            LexemeL)
// Lexeme:    Pointer to first character of Lexeme.
// LexemeEnd: Pointer to first character after Lexeme.
{
    // NOTE: For an explanation of the algorithm, see the function:
    //       count_indentation(...).
    QUEX_CHARACTER_TYPE* Begin = (QUEX_CHARACTER_TYPE*)Lexeme;

    // (1) Last character == newline ? _______________________________________________
    //     [impossible, lexeme does never contain a newline]
    // (2) Find last newline in lexeme _______________________________________________
    //     [cannot be found]
    // (2.1) no newline in lexeme? [yes]
    if( _indentation_count_enabled_f == false ) {
        // count increment without consideration of indentations
        // no newline => no line number increment
        //               no column number overflow / restart at '1'
        // no indentation enabled => no indentation increment
#       ifdef QUEX_OPTION_COLUMN_NUMBER_COUNTING
        _column_number_at_end += LexemeL;
#       endif
        return;
    }
    
    // The flag '_indentation_count_enabled_f' tells us that before this
    // pattern there was only whitespace after newline. Let us add the
    // whitespace at the beginning of this pattern to the _indentation.
    __count_whitespace_to_first_non_whitespace(Begin, Begin, Begin + LexemeL, 
                                               /* LicenseToIncrementLineCountF = */ false);

    __QUEX_LEXER_COUNT_ASSERT_CONSISTENCY();
}

inline void  
CounterWithIndentation::icount_NoNewline_NeverStartOnWhitespace(const int ColumnNIncrement) 
    // This is the fastest way to count: simply add the constant integer that represents 
    // the constant length of the lexeme (for patterns with fixed length, e.g. keywords).
{
    __quex_assert(ColumnNIncrement > 0);  // lexeme length >= 1
#   ifdef QUEX_OPTION_COLUMN_NUMBER_COUNTING
    _column_number_at_end += ColumnNIncrement;
#   endif
    if( _indentation_count_enabled_f ) {
        _indentation_count_enabled_f = false; 
        _the_lexer->mode().on_indentation(_the_lexer, _indentation);
    }
    __QUEX_LEXER_COUNT_ASSERT_CONSISTENCY();
}

inline void  
CounterWithIndentation::icount_NoNewline_ContainsOnlySpace(const int ColumnNIncrement) 
{
    __quex_assert(ColumnNIncrement > 0);  // lexeme length >= 1
#   ifdef QUEX_OPTION_COLUMN_NUMBER_COUNTING
    _column_number_at_end += ColumnNIncrement;
#   endif
    if( _indentation_count_enabled_f ) _indentation += ColumnNIncrement;

    __QUEX_LEXER_COUNT_ASSERT_CONSISTENCY();
}

inline void
CounterWithIndentation::__count_whitespace_to_first_non_whitespace(QUEX_CHARACTER_TYPE* start_consideration_it, 
                                                                   QUEX_CHARACTER_TYPE* Begin,
                                                                   QUEX_CHARACTER_TYPE* End,
                                                                   const bool           LicenseToCountF)
// NOTE: The 'license' flag shall enable the compiler to **delete** the line number counting
//       from the following function or implement it unconditionally, since the decision
//       is based on a constant (either true or false) -- once the function has been inlined.   
{
    // (3) Count _____________________________________________________________________
    //
    // -- whitespace from: start_consideration to first non-whitespace
    //    (indentation count is disabled if non-whitespace arrives)
    QUEX_CHARACTER_TYPE* it = start_consideration_it;
    do { 
        if( *it != ' ' ) { 
            _indentation_count_enabled_f = false;
            _indentation += it - start_consideration_it;
            // Line and column number need to be counted before the indentation handler
            // is called. this way it has to correct information.
            __count_indentation_aux(start_consideration_it, Begin, End, LicenseToCountF);
            // indentation event enabled:
            //   yes -> call indentation event handler
            //   no  -> enable event for the next time.
            //          indentation events can only be disabled for one coming event.
            if( _indentation_event_enabled_f ) 
                _the_lexer->mode().on_indentation(_the_lexer, _indentation);
            else
                // event was disabled this time, enable it for the next time.
                _indentation_event_enabled_f = true;

            return;
        }
        ++it; 		    
    } while ( it != End );

    // no non-whitespace until end of lexeme, thus only increment the indentation
    _indentation += it - start_consideration_it;
    __count_indentation_aux(start_consideration_it, Begin, End, LicenseToCountF);
}

inline void
CounterWithIndentation::__count_indentation_aux(QUEX_CHARACTER_TYPE* start_consideration_it,
                                                QUEX_CHARACTER_TYPE* Begin,
                                                QUEX_CHARACTER_TYPE* End, 
                                                const bool           LicenseToCountF)
{
    // when inlined, this is a condition on a constant => deleted by compiler.
    if( LicenseToCountF == false ) return;

#   ifdef  QUEX_OPTION_LINE_NUMBER_COUNTING
    __count_newline_n_backwards(start_consideration_it, Begin);
#   endif	    
#   ifdef  QUEX_OPTION_COLUMN_NUMBER_COUNTING
    _column_number_at_end += End - start_consideration_it;
#   endif

}

} // namespace quex 

#endif // __QUEX_OPTION_INDENTATION_TRIGGER_SUPPORT	
