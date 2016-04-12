/* -*- C++ -*- vim: set syntax=cpp: */
#ifndef __INCLUDE_GUARD_QUEX__CODE_BASE__BUFFER__BUFFER_FILLER_I__
#define __INCLUDE_GUARD_QUEX__CODE_BASE__BUFFER__BUFFER_FILLER_I__

#include <quex/code_base/definitions>
#include <quex/code_base/buffer/Buffer.i>
#include <quex/code_base/buffer/Buffer_debug.i>
#include <quex/code_base/buffer/BufferFiller>
#include <quex/code_base/buffer/plain/BufferFiller_Plain>
#ifdef QUEX_OPTION_ENABLE_ICONV
#   include <quex/code_base/buffer/iconv/BufferFiller_IConv>
#endif
#if ! defined(__QUEX_SETTING_PLAIN_C)
#   include <stdexcept>
namespace quex { 
#endif

    QUEX_INLINE void   __QuexBufferFiller_on_overflow(QuexBuffer*, bool ForwardF);
    QUEX_INLINE size_t __QuexBufferFiller_forward_copy_fallback_region(QuexBuffer*,
                                                                       const size_t Distance_LexemeStart_to_InputP);
    QUEX_INLINE void   __QuexBufferFiller_forward_adapt_pointers(QuexBuffer*, 
                                                                 const size_t DesiredLoadN,
                                                                 const size_t LoadedN,
                                                                 const size_t FallBackN, 
                                                                 const size_t Distance_LexemeStart_to_InputP);
    QUEX_INLINE size_t __QuexBufferFiller_backward_copy_backup_region(QuexBuffer*);
    QUEX_INLINE void   __QuexBufferFiller_backward_adapt_pointers(QuexBuffer*, 
                                                                          const size_t BackwardDistance);

#   ifndef __QUEX_SETTING_PLAIN_C
    /* NOTE: The filler types carry only pointers to input handles. So, the 
     *       required memory size is the same for any type. Let us use std::istream*
     *       to feed the sizeof()-operator. */
#   define BUFFER_FILLER_PLAIN   QuexBufferFiller_Plain<std::istream>
#   define BUFFER_FILLER_ICONV   QuexBufferFiller_IConv<std::istream>
#   else
#   define BUFFER_FILLER_PLAIN   QuexBufferFiller_Plain
#   define BUFFER_FILLER_ICONV   QuexBufferFiller_IConv
#   endif
    QUEX_INLINE size_t
    QuexBufferFiller_get_memory_size(QuexBufferFillerTypeEnum FillerType)
    {
        switch( FillerType ) {
        default:         QUEX_ERROR_EXIT("No memory size for QUEX_AUTO.\n");
        case QUEX_PLAIN: return sizeof(BUFFER_FILLER_PLAIN);
        case QUEX_ICONV: 
#           ifdef QUEX_OPTION_ENABLE_ICONV
            return sizeof(BUFFER_FILLER_ICONV);
#           else
            QUEX_ERROR_EXIT("Use of buffer filler type 'QUEX_ICONV' while option 'QUEX_OPTION_ENABLE_ICONV'\n" \
                            "is not specified. If defined, then the iconv-library must be installed on your system!\n");
            return -1;
#           endif
        }
    }
#   undef BUFFER_FILLER_PLAIN 
#   undef BUFFER_FILLER_ICONV 

    QUEX_INLINE void       
    QuexBufferFiller_destroy(QuexBufferFiller* me)
    { 
        /* if no dedicated deallocator is specified then free only the basic
         * BufferFiller structure. */
        if( me->_destroy == 0x0 ) __QUEX_FREE_MEMORY(me);
        else                      me->_destroy(me);
    }

    QUEX_INLINE void
    __QuexBufferFiller_init_functions(QuexBufferFiller* me,
                                      size_t       (*tell_character_index)(QuexBufferFiller*),
                                      void         (*seek_character_index)(QuexBufferFiller*, const size_t),
                                      size_t       (*read_characters)(QuexBufferFiller*,
                                                                      QUEX_CHARACTER_TYPE* buffer, const size_t),
                                      void         (*destroy)(QuexBufferFiller*))
    {
        __quex_assert(me != 0x0);
        __quex_assert(tell_character_index != 0x0);
        __quex_assert(seek_character_index != 0x0);
        __quex_assert(read_characters != 0x0);

        me->tell_character_index = tell_character_index;
        me->seek_character_index = seek_character_index;
        me->read_characters      = read_characters;
        me->_on_overflow         = 0x0;
        me->_destroy             = destroy;
    }

    QUEX_INLINE void
    QuexBufferFiller_initial_load(QuexBuffer* buffer)
    {
        const size_t         ContentSize  = QuexBuffer_content_size(buffer);
        QUEX_CHARACTER_TYPE* ContentFront = QuexBuffer_content_front(buffer);

        /* Assume: Buffer initialization happens independently */
        __quex_assert(buffer->_content_first_character_index == 0);
        __quex_assert(buffer->_input_p                       == ContentFront);   
        __quex_assert(buffer->_lexeme_start_p                == ContentFront);

        const size_t  LoadedN = buffer->filler->read_characters(buffer->filler, ContentFront, ContentSize);

        /* If end of file has been reached, then the 'end of file' pointer needs to be set*/
        if( LoadedN != ContentSize ) QuexBuffer_end_of_file_set(buffer, ContentFront + LoadedN);
        else                         QuexBuffer_end_of_file_unset(buffer);

        QUEX_BUFFER_ASSERT_CONTENT_CONSISTENCY(buffer);
    } 

    QUEX_INLINE size_t
    QuexBufferFiller_load_forward(QuexBuffer* buffer)
    {
        /* PURPOSE: This function is to be called as a reaction to a buffer limit code 'BLC'
         *          as returned by 'get_forward()'. Its task is to load new content into the 
         *          buffer such that 'get_forward() can continue iterating. This means that the 
         *          '_input_p' points to one of the following positions:
         *
         *          (1) Beginning of the Buffer: In this case, no reload needs to take place.
         *              It can basically only appear if 'load_forward()' is called after
         *              'get_backward()'---and this does not make sense. But returning a '0'
         *              (which is >= 0 and indicates that everything is ok) tells the application 
         *              that nothing has been loaded, and the next 'get_forward()' will work 
         *              normally.
         *
         *          (2) End of File Pointer: (which may be equal to the end of the buffer) 
         *              In this case no further content can be loaded. The function returns '0'.
         *
         *          (3) End of Buffer (if it is != End of File Pointer): Here a 'normal' load of
         *              new data into the buffer can happen.
         *
         * RETURNS: '>= 0'   number of characters that were loaded forward in the stream.
         *          '-1'     if forward loading was not possible (end of file)                      */
        /* 
         * NOTE: There is a seemingly dangerous case where the loading **just** fills the buffer to 
         *       the limit. In this case no 'End Of File' is detected, no end of file pointer is set,
         *       and as a consequence a new loading will happen later. This new loading, though,
         *       will only copy the fallback-region. The 'LoadedN == 0' will cause the _end_of_file_p
         *       to be set to the end of the copied fallback-region. And everything is fine.
         */
        QuexBufferFiller*  me = buffer->filler;
        if( me == 0x0 ) return 0; /* This case it totally rational, if no filler has been specified */

        __quex_assert(buffer != 0x0);
        __quex_assert(me->tell_character_index != 0x0);
        __quex_assert(me->seek_character_index != 0x0);
        __quex_assert(me->read_characters != 0x0);
        size_t       ContentSize = QuexBuffer_content_size(buffer);

        /* Catch impossible scenario: If the stretch from _input_p to _lexeme_start_p 
         * spans the whole buffer content, then nothing can be loaded into it.                      */
        const size_t Distance_LexemeStart_to_InputP = buffer->_input_p - buffer->_lexeme_start_p;
        if( Distance_LexemeStart_to_InputP >= ContentSize ) { 
            __QuexBufferFiller_on_overflow(buffer, /* Forward */ true);
            return 0;
        }
        QUEX_DEBUG_PRINT_BUFFER_LOAD(buffer, "FORWARD(entry)");

        /* (*) Check for the three possibilities mentioned above */
        if     ( buffer->_input_p == buffer->_memory._front ) { return 0; }      /* (1)*/
        else if( buffer->_input_p == buffer->_end_of_file_p ) { return 0; }      /* (2)*/
        else if( buffer->_input_p != buffer->_memory._back  ) {                     
            QUEX_ERROR_EXIT("Call to 'load_forward() but '_input_p' not on buffer border.\n" 
                            "(Check character encoding)");  
        }
        else if( buffer->_end_of_file_p != 0x0 ) { 
            /* End of file has been reached before, we cannot load more.               */
            return 0;                               
        }
        /* HERE: _input_p ---> LAST ELEMENT OF THE BUFFER!                        * (3)*/  

        /*___________________________________________________________________________________*/
        /* (1) Handle fallback region*/
        const size_t FallBackN = __QuexBufferFiller_forward_copy_fallback_region(buffer, 
                                                                                 Distance_LexemeStart_to_InputP);
        __quex_assert(FallBackN < ContentSize);
        /*___________________________________________________________________________________*/
        /* (2) Load new content*/
        /* NOTE: Due to backward loading the character index might not stand on the end of
         *       the buffer. Thus a seek is necessary.                                       */
        const size_t CharacterIndexAtEnd = (size_t)(buffer->_content_first_character_index + 
                                                    (QuexBuffer_content_size(buffer)));
        me->seek_character_index(me, CharacterIndexAtEnd);

        const size_t         DesiredLoadN = ContentSize - FallBackN;
        QUEX_CHARACTER_TYPE* new_content_begin = buffer->_memory._front + 1 + FallBackN;
        const size_t         LoadedN           = me->read_characters(me, new_content_begin, DesiredLoadN);

        /*___________________________________________________________________________________*/
        /* (3) Adapt the pointers in the buffer*/
        __QuexBufferFiller_forward_adapt_pointers(buffer, 
                                                  DesiredLoadN, LoadedN, FallBackN, 
                                                  Distance_LexemeStart_to_InputP);

        QUEX_DEBUG_PRINT_BUFFER_LOAD(buffer, "LOAD FORWARD(exit)");
        /* NOTE: During backward loading the rule does not hold that the stream position
         *       corresponds to the last character in the buffer. The last character in the
         *       buffer might be a copy from the front of the buffer. Thus, this constraint
         *       only holds **after the forwad load**.                                       */
#       ifdef QUEX_OPTION_ASSERTS
        const size_t   ComputedCharacterIndexAtEnd = (size_t)(buffer->_content_first_character_index + 
                                                     (QuexBuffer_text_end(buffer) - QuexBuffer_content_front(buffer)));
        const size_t   RealCharacterIndexAtEnd     = buffer->filler->tell_character_index(buffer->filler);
        __quex_assert( ComputedCharacterIndexAtEnd == RealCharacterIndexAtEnd );
#       endif
        QUEX_BUFFER_ASSERT_CONSISTENCY(buffer);
        QUEX_BUFFER_ASSERT_CONTENT_CONSISTENCY(buffer);

        /* NOTE: Return value is used for adaptions of memory addresses. It happens that the*/
        /*       address offset is equal to DesiredLoadN; see function __forward_adapt_pointers().*/
        return DesiredLoadN; /* THUS NOT: LoadedN*/
    }

    QUEX_INLINE size_t
    __QuexBufferFiller_forward_copy_fallback_region(QuexBuffer* buffer,
                                                    const size_t Distance_LexemeStart_to_InputP)
    {
        /* (1) Fallback: A certain region of the current buffer is copied in front such that
         *               if necessary the stream can go backwards without a backward load.
         *
         *                            fallback_n
         *                               :
         *                |11111111111111:22222222222222222222222222222222222222|
         *                  copy of      :   new loaded content of buffer
         *                  end of old   
         *                  buffer      
         *
         *     The fallback region is related to the lexeme start pointer. The lexeme start 
         *     pointer always needs to lie inside the buffer, because applications might read
         *     their characters from it. The 'stretch' [lexeme start, current_p] must be 
         *     contained in the new buffer (precisely in the fallback region). */
        QUEX_BUFFER_ASSERT_CONSISTENCY(buffer);
        __quex_assert((int)Distance_LexemeStart_to_InputP == buffer->_input_p - buffer->_lexeme_start_p);
        __quex_assert(Distance_LexemeStart_to_InputP < QuexBuffer_content_size(buffer));
        /* Copying forward shall **only** happen when new content is to be loaded. This is not the case
         * if EOF as reached and the _end_of_file_p lies inside the buffer. Thus the _input_p
         * must have reached the upper border of the buffer. */
        __quex_assert(buffer->_input_p == buffer->_memory._back);

        /* (*) Fallback region = max(default size, necessary size)*/
        const size_t FallBackN = QUEX_SETTING_BUFFER_MIN_FALLBACK_N > Distance_LexemeStart_to_InputP 
                                 ? QUEX_SETTING_BUFFER_MIN_FALLBACK_N  
                                 : Distance_LexemeStart_to_InputP;

        /* (*) Copy fallback region*/
        /*     If there is no 'overlap' from source and drain than the faster memcpy() can */
        /*     used instead of memmove().*/
        QUEX_CHARACTER_TYPE*  source = QuexBuffer_content_back(buffer) - FallBackN + 1; /* end of content - fallback*/
        QUEX_CHARACTER_TYPE*  drain  = QuexBuffer_content_front(buffer);       
        /* Cast to uint8_t to avoid that some smart guy provides a C++ overloading function */
        if( drain + FallBackN >= source  ) {
            __QUEX_STD_memmove((uint8_t*)drain, (uint8_t*)source, FallBackN * sizeof(QUEX_CHARACTER_TYPE));
        } else { 
            __QUEX_STD_memcpy((uint8_t*)drain, (uint8_t*)source, FallBackN * sizeof(QUEX_CHARACTER_TYPE));
        }

#       ifdef QUEX_OPTION_ASSERTS
        /* Cast to uint8_t to avoid that some smart guy provides a C++ overloading function */
        __QUEX_STD_memset((uint8_t*)(drain + FallBackN), (uint8_t)(0xFF), 
                          (QuexBuffer_content_size(buffer) - FallBackN)*sizeof(QUEX_CHARACTER_TYPE)); 
#       endif

        __quex_assert(FallBackN < QuexBuffer_content_size(buffer));
        return FallBackN;
    }

    QUEX_INLINE void
    __QuexBufferFiller_forward_adapt_pointers(QuexBuffer* buffer, 
                                              const size_t DesiredLoadN,
                                              const size_t LoadedN,
                                              const size_t FallBackN, 
                                              const size_t Distance_LexemeStart_to_InputP)
    {
        const size_t         ContentSize  = QuexBuffer_content_size(buffer);
        QUEX_CHARACTER_TYPE* ContentFront = QuexBuffer_content_front(buffer);

        __quex_assert( buffer->_end_of_file_p == 0x0 || LoadedN + FallBackN == ContentSize );
        __quex_assert( DesiredLoadN != 0 );

        /* (*) If end of file has been reached, then the 'end of file' pointer needs to be set*/
        if( LoadedN != DesiredLoadN ) 
            QuexBuffer_end_of_file_set(buffer, ContentFront + FallBackN + LoadedN);
        else
            QuexBuffer_end_of_file_unset(buffer);


        /* (*) Character index of the first character in the content of the buffer  
         *     increases by content size - fallback indenpendently how many bytes  
         *     have actually been loaded. */
        buffer->_content_first_character_index += ContentSize - FallBackN;

        /*___________________________________________________________________________________*/
        /* (*) Pointer adaption*/
        /*     Next char to be read: '_input_p + 1'*/
        buffer->_input_p        = ContentFront + FallBackN - 1;   
        /*     NOTE: _input_p is set to (_input_p - 1) so that the next *(++_input_p) */
        /*           reads the _input_p.*/
        buffer->_lexeme_start_p = (buffer->_input_p + 1) - Distance_LexemeStart_to_InputP; 

        __quex_assert( buffer->_end_of_file_p == 0x0 || (int)(LoadedN + FallBackN) == buffer->_end_of_file_p - buffer->_memory._front - 1);

    }


    QUEX_INLINE size_t   
    QuexBufferFiller_load_backward(QuexBuffer* buffer)
    {
        QuexBufferFiller* me = buffer->filler;
        QUEX_BUFFER_ASSERT_CONSISTENCY(buffer);

        if( me == 0x0 ) return 0; /* This case it totally rational, if no filler has been specified */

        /* PURPOSE: This function is to be called as a reaction to a buffer limit code 'BLC'
         *          as returned by 'get_backward()'. Its task is the same as the one of 
         *          'load_forward()'--only in opposite direction. Here only two cases need 
         *          to be distinguished. The current_p points to 
         *
         *          (1) End of Buffer or End of File pointer: No backward load needs to 
         *              happen. This can only occur if a 'get_forward()' was called right
         *              before.
         *
         *          (2) Begin of the buffer and the buffer is the 'start buffer':
         *              in this case no backward load can happen, because we are at the 
         *              beginning. The function returns 0.
         *
         *          (3) Begin of buffer and _begin_of_file_f is not set!: This is the case
         *              where this function, actually, has some work to do. It loads the
         *              buffer with 'earlier' content from the file.
         *
         *
         * RETURNS: Distance that was loaded backwards.
         *          -1 in case of backward loading is not possible (begin of file)
         *     
         * COMMENT: 
         *     
         * For normal cases the fallback region, i.e. the 'FALLBACK_N' buffer bytes 
         * allows to go a certain distance backwards immediately. If still the begin 
         * of the buffer is reached, then this is an indication that something is
         * 'off-the-norm'. Lexical analysis is not supposed to go longtimes
         * backwards. For such cases we step a long stretch backwards: A
         * THIRD of the buffer's size! 
         *
         * A meaningful fallback_n would be 64 Bytes. If the buffer's size
         * is for example 512 kB then the backwards_distance of A THIRD means 170
         * kB. This leaves a  safety region which is about 2730 times
         * greater than normal (64 Bytes). After all, lexical analysis means
         * to go **mainly forward** and not backwards.  */
        __quex_assert(buffer != 0x0);
        QUEX_CHARACTER_TYPE* ContentFront = QuexBuffer_content_front(buffer);
        QUEX_CHARACTER_TYPE* ContentBack  = QuexBuffer_content_back(buffer);

        QUEX_DEBUG_PRINT_BUFFER_LOAD(buffer, "BACKWARD(entry)");
        QUEX_BUFFER_ASSERT_CONSISTENCY(buffer);

        /* (*) Check for the three possibilities mentioned above*/
        if     ( buffer->_input_p == buffer->_memory._back )  { return 0; }   /* (1) */
        else if( buffer->_input_p == buffer->_end_of_file_p ) { return 0; }   /* (1) */
        else if( buffer->_input_p != buffer->_memory._front ) {
            QUEX_ERROR_EXIT("Call to 'load_backward() but '_input_p' not on buffer border.\n" 
                            "(Check character encoding)");  
        }
        else if( buffer->_content_first_character_index == 0 ) { return 0; }  /* (2) */
        /*                                                                     * (3) */
        /* HERE: current_p == FRONT OF THE BUFFER!   
         *_______________________________________________________________________________*/
        /* Catch impossible scenario: If the stretch from _input_p to _lexeme_start_p 
         * spans the whole buffer content, then nothing can be loaded into it. */
        if( buffer->_lexeme_start_p == ContentBack ) { 
            __QuexBufferFiller_on_overflow(buffer, /* ForwardF */ false);
            return 0;
        }
        /* (1) Compute distance to go backwards*/
        const size_t BackwardDistance = __QuexBufferFiller_backward_copy_backup_region(buffer);
        if( BackwardDistance == 0 ) return 0;

        /*_______________________________________________________________________________
         * (2) Compute the stream position of the 'start to read'   
         *  
         * It is not safe to assume that the character size is fixed. Thus it is up to  
         * the input strategy to determine the input position that belongs to a character  
         * position. */
        __quex_assert( buffer->_content_first_character_index >= BackwardDistance );
        const size_t NewContentFirstCharacterIndex = buffer->_content_first_character_index - BackwardDistance;

        /*_______________________________________________________________________________
         * (3) Load content*/
        me->seek_character_index(me, NewContentFirstCharacterIndex);
#       ifdef QUEX_OPTION_ASSERTS
        const size_t LoadedN = /* avoid unused variable in case '__quex_assert()' is deactivated*/
#       endif
        /* -- If file content < buffer size, then the start position of the stream to which  
         *    the buffer refers is always 0 and no backward loading will ever happen.  
         * -- If the file content >= buffer size, then backward loading must always fill  
         *    the buffer. */
        me->read_characters(me, ContentFront, BackwardDistance);

        __quex_assert(LoadedN == (size_t)BackwardDistance);

        /*________________________________________________________________________________
         * (4) Adapt pointers*/
        __QuexBufferFiller_backward_adapt_pointers(buffer, BackwardDistance);

        QUEX_DEBUG_PRINT_BUFFER_LOAD(buffer, "BACKWARD(exit)");
        QUEX_BUFFER_ASSERT_CONSISTENCY(buffer);
        QUEX_BUFFER_ASSERT_CONTENT_CONSISTENCY(buffer);

        return BackwardDistance;
    }


    QUEX_INLINE size_t
    __QuexBufferFiller_backward_copy_backup_region(QuexBuffer* buffer)
    {
        const size_t         ContentSize  = QuexBuffer_content_size(buffer);
        QUEX_CHARACTER_TYPE* ContentFront = QuexBuffer_content_front(buffer);

        QUEX_BUFFER_ASSERT_CONSISTENCY(buffer);
        /* Copying backward shall **only** happen when new content is to be loaded. In back
         * ward direction, this makes only sense if the lower border was reached. */
        __quex_assert(buffer->_input_p == buffer->_memory._front);
        /* We need to make sure, that the lexeme start pointer remains inside the
         * buffer, so that we do not loose the reference. From current_p == buffer begin
         * it is safe to say that _lexeme_start_p > _input_p (the lexeme starts
         * on a letter not the buffer limit). */
        __quex_assert(buffer->_lexeme_start_p > buffer->_input_p);
        __quex_assert((size_t)(buffer->_lexeme_start_p - buffer->_input_p) < QuexBuffer_content_size(buffer));

        const size_t IntendedBackwardDistance = (size_t)(ContentSize / 3);   

        /* Limit 1:
         *
         *     Before:    |C      L                  |
         *
         *     After:     |       C      L           |
         *                 ------->
         *                 backward distance
         *
         *     Lexeme start pointer L shall lie inside the buffer. Thus, it is required:
         *
         *               backward distance + (C - L) < size
         *           =>            backward distance < size - (C - L)
         *          */
        /*           (result is possitive, see __quex_assert(...) above) */
        const size_t Distance_InputP_to_LexemeStart = (size_t)(buffer->_lexeme_start_p - buffer->_input_p);
        const int    LimitBackwardDist_1 = ContentSize - Distance_InputP_to_LexemeStart; 

        /* Limit 2:
         *     We cannot go before the first character in the stream. */
        const int    LimitBackwardDist_2 = (int)buffer->_content_first_character_index;

        /* Taking the minimum of all:*/
        const size_t Limit_1_and_2 = LimitBackwardDist_1 < LimitBackwardDist_2 ? LimitBackwardDist_1
                                     :                                           LimitBackwardDist_2;
        const size_t BackwardDistance = IntendedBackwardDistance < Limit_1_and_2 ?  IntendedBackwardDistance 
                                        :                                           Limit_1_and_2;

        /* (*) copy content that is already there to its new position.
         *     (copying is much faster then loading new content from file). */
        __QUEX_STD_memmove(ContentFront + BackwardDistance, ContentFront, 
                           (ContentSize - BackwardDistance)*sizeof(QUEX_CHARACTER_TYPE));

#       ifdef QUEX_OPTION_ASSERTS
        /* Cast to uint8_t to avoid that some smart guy provides a C++ overloading function */
        __QUEX_STD_memset((uint8_t*)ContentFront, (uint8_t)(0xFF), BackwardDistance * sizeof(QUEX_CHARACTER_TYPE)); 
#       endif
        return BackwardDistance;
    }

    QUEX_INLINE void
    __QuexBufferFiller_backward_adapt_pointers(QuexBuffer* buffer, const size_t BackwardDistance)
    {
        /* -- end of file / end of buffer:*/
        if( buffer->_end_of_file_p ) {
            QUEX_CHARACTER_TYPE*   NewEndOfFileP = buffer->_end_of_file_p + BackwardDistance;
            if( NewEndOfFileP <= buffer->_memory._back ) 
                QuexBuffer_end_of_file_set(buffer, NewEndOfFileP);
            else  
                QuexBuffer_end_of_file_unset(buffer);
        }
        /* -- character index of begin of buffer = where we started reading new content*/
        buffer->_content_first_character_index -= BackwardDistance;

        buffer->_input_p        += BackwardDistance + 1; 
        buffer->_lexeme_start_p += BackwardDistance;
    }

    QUEX_INLINE void
    __QuexBufferFiller_on_overflow(QuexBuffer* buffer, bool ForwardF)
    {
        QuexBufferFiller* me = buffer->filler;

        if(    me->_on_overflow == 0x0
            || me->_on_overflow(buffer, ForwardF) == false ) {

#           ifdef QUEX_OPTION_INFORMATIVE_BUFFER_OVERFLOW_MESSAGE
            /* Print out the lexeme start, so that the user has a hint. */
            uint8_t               utf8_encoded_str[512]; 
            static char           message[1024];
            const size_t          MessageSize = 1024;
            uint8_t*              WEnd        = utf8_encoded_str + 512 - 7;
            uint8_t*              witerator   = utf8_encoded_str; 
            QUEX_CHARACTER_TYPE*  End         = buffer->_memory._back; 
            QUEX_CHARACTER_TYPE*  iterator    = buffer->_lexeme_start_p; 
            int                   utf8_length = 0;
            
            for(; witerator < WEnd &&  iterator != End ; ++iterator) {
                utf8_length = Quex_unicode_to_utf8(*iterator, witerator);
                if( utf8_length < 0 || utf8_length > 6) continue;
                witerator += utf8_length;
                *witerator = '\0';
            }
            message[0] = '\0';
            /* No use of 'snprintf()' because not all systems seem to support it propperly. */
            __QUEX_STD_strncat(message, 
                               "Distance between lexeme start and current pointer exceeds buffer size.\n"
                               "(tried to load buffer",
                               MessageSize);
            __QUEX_STD_strncat(message, ForwardF ? "forward)" : "backward)",                   MessageSize);
            __QUEX_STD_strncat(message, "As a hint consider the beginning of the lexeme:\n[[", MessageSize);
            __QUEX_STD_strncat(message, (char*)utf8_encoded_str,                               MessageSize);
            __QUEX_STD_strncat(message, "]]\n",                                                MessageSize);

            QUEX_ERROR_EXIT(message);
#           else
            QUEX_ERROR_EXIT("Distance between lexeme start and current pointer exceeds buffer size.\n"
                            "(tried to load buffer forward). Please, compile with option\n\n"
                            "    QUEX_OPTION_INFORMATIVE_BUFFER_OVERFLOW_MESSAGE\n\n"
                            "in order to get a more informative output. Most likely, one of your patterns\n"
                            "eats longer as you inteded it. Alternatively you might want to set the buffer\n"
                            "size to a greate value or use skippers (<skip: [ \n\t]> for example.");
#           endif /* QUEX_OPTION_INFORMATIVE_BUFFER_OVERFLOW_MESSAGE */
        }
    }

#if ! defined(__QUEX_SETTING_PLAIN_C)
} // namespace quex
#endif

#endif /* __INCLUDE_GUARD_QUEX__CODE_BASE__BUFFER__BUFFER_FILLER_I__ */

