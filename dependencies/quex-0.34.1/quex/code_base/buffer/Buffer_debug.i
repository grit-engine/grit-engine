/* -*- C++ -*-  vim: set syntax=cpp:
 *
 * (C) 2008 Frank-Rene Schaefer */
#ifndef __INCLUDE_GUARD_QUEX__CODE_BASE__BUFFER__BUFFER_DEBUG_I_
#define __INCLUDE_GUARD_QUEX__CODE_BASE__BUFFER__BUFFER_DEBUG_I_

#include <quex/code_base/definitions>
#include <quex/code_base/buffer/Buffer>
#include <quex/code_base/buffer/BufferFiller>



#if ! defined (__QUEX_SETTING_PLAIN_C)
namespace quex {
#endif

    QUEX_INLINE void BufferFiller_x_show_content(QuexBuffer* buffer); 
    QUEX_INLINE void BufferFiller_show_brief_content(QuexBuffer* buffer);
    QUEX_INLINE void BufferFiller_show_content(QuexBuffer* buffer); 

    QUEX_INLINE void 
    BufferFiller_show_brief_content(QuexBuffer* buffer) 
    {
        __quex_assert(buffer != 0x0);
        QuexBufferFiller* me = buffer->filler;
        __quex_assert(me != 0x0);

        QUEX_BUFFER_ASSERT_CONSISTENCY(buffer);
        __QUEX_STD_printf("Begin of Buffer Character Index: %i\n", (int)buffer->_content_first_character_index);
        __QUEX_STD_printf("End   of Buffer Character Index: %i\n", (int)me->tell_character_index(me));
        if( buffer->_end_of_file_p == 0x0 )
            __QUEX_STD_printf("_end_of_file_p (offset)  = <0x0>\n");
        else
            __QUEX_STD_printf("_end_of_file_p (offset)  = %08X\n", 
                              (int)(buffer->_end_of_file_p  - buffer->_memory._front));
        __QUEX_STD_printf("_input_p (offset)        = %08X\n",     (int)(buffer->_input_p        - buffer->_memory._front));
        __QUEX_STD_printf("_lexeme_start_p (offset) = %08X\n",     (int)(buffer->_lexeme_start_p - buffer->_memory._front));
        __QUEX_STD_printf("_back (offset)           = %08X\n",     (int)(buffer->_memory._back   - buffer->_memory._front));
    }

    QUEX_INLINE void 
    BufferFiller_x_show_content(QuexBuffer* buffer) 
    {
        BufferFiller_show_content(buffer);
        BufferFiller_show_brief_content(buffer);
    }

    QUEX_INLINE QUEX_CHARACTER_TYPE
    __BufferFiller_get_border_char(QuexBuffer* buffer, const QUEX_CHARACTER_TYPE* C) 
    {
        if     ( *C != QUEX_SETTING_BUFFER_LIMIT_CODE )   
            return (QUEX_CHARACTER_TYPE)'?'; 
        else if( buffer->_end_of_file_p == C )       
            return (QUEX_CHARACTER_TYPE)']';
        else if( buffer->_content_first_character_index == 0 && buffer->_memory._front == C )     
            return (QUEX_CHARACTER_TYPE)'[';
        else
            return (QUEX_CHARACTER_TYPE)'|';
    }

    QUEX_INLINE void
    QuexBuffer_show_content(QuexBuffer* buffer)
    {
#       if QUEX_CHARACTER_TYPE   == uint8_t
#          define QUEX_BUFFER_EMPTY_CHAR ((uint8_t)0xFFFFFFFF)
#       elif QUEX_CHARACTER_TYPE == uint16_t
#          define QUEX_BUFFER_EMPTY_CHAR ((uint16_t)0xFFFFFFFF)
#       else /* QUEX_CHARACTER_TYPE == uint32_t */
#          define QUEX_BUFFER_EMPTY_CHAR ((uint32_t)0xFFFFFFFF)
#       endif
        size_t                i = 0;
        QUEX_CHARACTER_TYPE   EmptyChar =   sizeof(QUEX_CHARACTER_TYPE) == 1 ? (QUEX_CHARACTER_TYPE)(0xFF)
                                          : sizeof(QUEX_CHARACTER_TYPE) == 2 ? (QUEX_CHARACTER_TYPE)(0xFFFF)
                                          :                                    (QUEX_CHARACTER_TYPE)(0xFFFFFFFF);
        QUEX_CHARACTER_TYPE*  ContentFront = QuexBuffer_content_front(buffer);
        QUEX_CHARACTER_TYPE*  BufferFront  = buffer->_memory._front;
        QUEX_CHARACTER_TYPE*  BufferBack   = buffer->_memory._back;
        QUEX_CHARACTER_TYPE*  iterator = 0x0;
        QUEX_CHARACTER_TYPE*  end_p    = buffer->_end_of_file_p != 0x0 ? buffer->_end_of_file_p 
                                         :                               buffer->_memory._back;

        __QUEX_STD_printf("|%c", __BufferFiller_get_border_char(buffer, BufferFront));
        for(iterator = ContentFront; iterator != end_p; ++iterator) {
            __QUEX_STD_printf("%c", *iterator == EmptyChar ? '~' : *iterator);
        }
        __QUEX_STD_printf("%c", __BufferFiller_get_border_char(buffer, end_p));
        /**/
        const size_t L = (buffer->_end_of_file_p == 0x0) ? 0 : BufferBack - buffer->_end_of_file_p;
        for(i=0; i < L; ++i) __QUEX_STD_printf("|");

        __QUEX_STD_printf("|");
    }

    QUEX_INLINE void  
    BufferFiller_show_content(QuexBuffer* buffer) 
    {
        __quex_assert(buffer != 0x0);
        /* NOTE: If the limiting char needs to be replaced temporarily by
         *       a terminating zero.
         * NOTE: This is a **simple** printing function for unit testing and debugging
         *       it is thought to print only ASCII characters (i.e. code points < 0xFF)*/
        size_t                i = 0;
        const size_t          ContentSize  = QuexBuffer_content_size(buffer);
        QUEX_CHARACTER_TYPE*  ContentFront = QuexBuffer_content_front(buffer);
        QUEX_CHARACTER_TYPE*  BufferFront  = buffer->_memory._front;
        QUEX_CHARACTER_TYPE*  BufferBack   = buffer->_memory._back;

        /*_________________________________________________________________________________*/
        char* tmp = (char*)__QUEX_ALLOCATE_MEMORY(ContentSize + 4);
        /* tmp[0]                 = outer border*/
        /* tmp[1]                 = buffer limit*/
        /* tmp[2...ContentSize+1] = ContentFront[0...ContentSize-1]*/
        /* tmp[ContentSize+2]     = buffer limit*/
        /* tmp[ContentSize+3]     = outer border*/
        /* tmp[ContentSize+4]     = terminating zero*/
        for(i=2; i<ContentSize + 2 ; ++i) tmp[i] = ' ';
        tmp[ContentSize+4] = '\0';
        tmp[ContentSize+3] = '|';
        tmp[ContentSize+2] = __BufferFiller_get_border_char(buffer, BufferBack);
        tmp[1]             = __BufferFiller_get_border_char(buffer, BufferFront);
        tmp[0]             = '|';
        /* tmp[_SHOW_current_fallback_n - 1 + 2] = ':';        */
        tmp[buffer->_input_p - ContentFront + 2] = 'C';
        if( buffer->_lexeme_start_p >= ContentFront && buffer->_lexeme_start_p <= BufferBack ) 
            tmp[(int)(buffer->_lexeme_start_p - ContentFront) + 2] = 'S';
        /**/
        if ( buffer->_input_p == ContentFront - 2 ) {
            __QUEX_STD_printf(tmp); __QUEX_STD_printf(" <out>");
        } else {
            __QUEX_STD_printf(" ");
            if( *buffer->_input_p == QUEX_SETTING_BUFFER_LIMIT_CODE ) 
                __QUEX_STD_printf("BLC");
            else                                  
                __QUEX_STD_printf("'%c'", (char)(*buffer->_input_p));
        }
        /* std::cout << " = 0x" << std::hex << int(*buffer->_input_p) << std::dec */
        __QUEX_STD_printf("\n");
        QuexBuffer_show_content(buffer);
        __QUEX_FREE_MEMORY(tmp);
    }

    QUEX_INLINE void  
    QuexBuffer_show_byte_content(QuexBuffer* buffer, const int IndentationN) 
    {
        __quex_assert(buffer != 0x0);
        QuexBufferMemory*  memory = &buffer->_memory;
        __quex_assert(memory != 0x0);

        int      i = 0, j = 0;
        uint8_t* byte_p      = (uint8_t*)memory->_front;
        uint8_t* next_byte_p = (uint8_t*)memory->_front + 1;
        uint8_t* End         = (uint8_t*)(memory->_back + 1);

        for(j=0; j<IndentationN; ++j) fprintf(stdout, " ");
        for(; byte_p != End; ++byte_p, ++next_byte_p, ++i) {
            fprintf(stdout, "%02X", (int)*byte_p);
            if     ( next_byte_p == (uint8_t*)buffer->_end_of_file_p ) 
                fprintf(stdout, "[");
            else if( byte_p      == (uint8_t*)buffer->_end_of_file_p + sizeof(QUEX_CHARACTER_TYPE)-1) 
                fprintf(stdout, "]");
            else 
                fprintf(stdout, ".");
            if( (i+1) % 0x8  == 0 ) fprintf(stdout, " ");
            if( (i+1) % 0x10 == 0 ) { 
                fprintf(stdout, "\n");
                for(j=0; j<IndentationN; ++j) fprintf(stdout, " ");
            }
        }
        fprintf(stdout, "\n");
    }

#if ! defined(__QUEX_SETTING_PLAIN_C)
} /* namespace quex */
#endif 

#include <quex/code_base/buffer/Buffer.i>


#endif /* __INCLUDE_GUARD_QUEX__CODE_BASE__BUFFER__BUFFER_DEBUG_I_ */
