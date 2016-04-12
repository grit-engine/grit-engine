/* -*- C++ -*- vim: set syntax=cpp: */
#ifndef __INCLUDE_GUARD__QUEX__CODE_BASE__ANALYSER_MINIMAL_I__
#define __INCLUDE_GUARD__QUEX__CODE_BASE__ANALYSER_MINIMAL_I__

#include <quex/code_base/definitions>
#include <quex/code_base/buffer/Buffer>

#if ! defined(__QUEX_SETTING_PLAIN_C)
namespace quex {
#endif

    TEMPLATE_IN(InputHandleT) void
    QuexAnalyser_construct(QuexAnalyser* me,
                           QUEX_ANALYSER_FUNCTION_TYPE  AnalyserFunction,
                           InputHandleT*                input_handle,
                           QuexBufferFillerTypeEnum     InputCodingType,
                           const char*                  IANA_InputCodingName, 
                           const size_t                 BufferMemorySize,
                           const size_t                 TranslationBufferMemorySize)
    /* input_handle == 0x0 means that there is no stream/file to read from. Instead, the 
     *                     user intends to perform the lexical analysis directly on plain
     *                     memory. In this case, the user needs to call the following function
     *                     by hand in order to setup the memory:
     *
     *                     QuexBufferMemory_init(analyse->buffer._memory, (uint8_t*)MyMemoryP, MyMemorySize); 
     */
    {
        QuexBuffer_construct(&me->buffer, 
                             input_handle,
                             InputCodingType, IANA_InputCodingName,
                             BufferMemorySize,
                             TranslationBufferMemorySize);

        me->current_analyser_function = AnalyserFunction;

        /* Double check that everything is setup propperly. */
        QUEX_BUFFER_ASSERT_CONSISTENCY(&me->buffer);
        __quex_assert(me->buffer._input_p == me->buffer._memory._front + 1);
    }

    QUEX_INLINE void
    QuexAnalyser_destruct(QuexAnalyser* me)
    {
        QuexBuffer_destruct(&me->buffer);
    }

    QUEX_INLINE void
    QuexAnalyser_reset(QuexAnalyser* me)
    {
        QuexBuffer_reset(&me->buffer);
    }

    /* NOTE: 'reload_forward()' needs to be implemented for each mode, because
     *       addresses related to acceptance positions need to be adapted. This
     *       is not the case for 'reload_backward()'. In no case of backward
     *       reloading, there are important addresses to keep track. */
    QUEX_INLINE bool 
    QuexAnalyser_buffer_reload_backward(QuexBuffer* buffer)
    {
        if( buffer->filler == 0x0 ) return false;

        const size_t LoadedCharacterN = QuexBufferFiller_load_backward(buffer);
        if( LoadedCharacterN == 0 ) return false;
        
        /* Backward lexing happens in two cases:
         *
         *  (1) When checking for a pre-condition. In this case, no dedicated acceptance
         *      is involved. No acceptance positions are considered.
         *  (2) When tracing back to get the end of a core pattern in pseudo-ambigous
         *      post conditions. Then, no acceptance positions are involved, because
         *      the start of the lexeme shall not drop before the begin of the buffer 
         *      and the end of the core pattern, is of course, after the start of the 
         *      lexeme. => there will be no reload backwards. */
        return true;
    }

    QUEX_INLINE bool 
    QuexAnalyser_buffer_reload_forward(QuexBuffer* buffer, 
                                       QUEX_CHARACTER_POSITION_TYPE* last_acceptance_input_position,
                                       QUEX_CHARACTER_POSITION_TYPE* post_context_start_position,
                                       const size_t                  PostContextN)
    {
        QUEX_CHARACTER_POSITION_TYPE* iterator = 0x0;
        QUEX_CHARACTER_POSITION_TYPE* End = post_context_start_position + PostContextN;

        if( buffer->filler == 0x0 ) return false;
        if( buffer->_end_of_file_p != 0x0 ) return false;
        const size_t LoadedCharacterN = QuexBufferFiller_load_forward(buffer);
        if( LoadedCharacterN == 0 ) return false;

        if( *last_acceptance_input_position != 0x0 ) { 
            *last_acceptance_input_position -= LoadedCharacterN;
            /* QUEX_DEBUG_ADR_ASSIGNMENT("last_acceptance_input_position", *last_acceptance_input_position); */
        }                                                                  
        for(iterator = post_context_start_position; iterator != End; ++iterator) {
            /* NOTE: When the post_context_start_position is still undefined the following operation may
             *       underflow. But, do not care, once it is **assigned** to a meaningful value, it won't */
            *iterator -= LoadedCharacterN;
        }
                                                                              
        return true;
    }


#if ! defined(__QUEX_SETTING_PLAIN_C)
} // namespace quex 
#endif

#include <quex/code_base/buffer/Buffer.i>
#include <quex/code_base/buffer/BufferFiller.i>
#include <quex/code_base/buffer/plain/BufferFiller_Plain.i>
#ifdef QUEX_OPTION_ENABLE_ICONV
#   include <quex/code_base/buffer/iconv/BufferFiller_IConv.i>
#endif

#endif /* __INCLUDE_GUARD__QUEX__CODE_BASE__ANALYSER_MINIMAL_I__ */
