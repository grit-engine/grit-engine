/* -*- C++ -*-  vim: set syntax=cpp:
 * (C) 2007-2008 Frank-Rene Schaefer  */
#ifndef __INCLUDE_GUARD__QUEX_BUFFER_INPUT_STRATEGY_ICONV_I__
#define __INCLUDE_GUARD__QUEX_BUFFER_INPUT_STRATEGY_ICONV_I_

#include <quex/code_base/buffer/BufferFiller>
#include <quex/code_base/buffer/iconv/BufferFiller_IConv>
#include <quex/code_base/buffer/InputPolicy>
#include <quex/code_base/MemoryManager>
#include <quex/code_base/compatibility/iconv-argument-types.h>


#ifdef QUEX_OPTION_ASSERTS
#   define QUEX_ASSERT_BUFFER_INFO(BI)                                        \
    __quex_assert( BI != 0x0 );                                               \
    __quex_assert((BI)->base.iterator >= (BI)->base.begin);                   \
    __quex_assert((BI)->fill_level_n <= (BI)->base.size);                     \
    __quex_assert((size_t)((BI)->base.iterator - (BI)->base.begin) <= (BI)->base.size); \
    __quex_assert((BI)->base.bytes_left_n <= (BI)->base.size);                          \
    __quex_assert((BI)->base.bytes_left_n <= (BI)->fill_level_n);                       \
    __quex_assert(   (size_t)((BI)->base.iterator - (BI)->base.begin)                   \
                  == (size_t)((BI)->fill_level_n  - (BI)->base.bytes_left_n));

#   define QUEX_ASSERT_BUFFER_INFO_EASY(BI)                              \
    __quex_assert( BI != 0x0 );                                          \
    __quex_assert((BI)->iterator >= (BI)->begin);                        \
    __quex_assert((size_t)((BI)->iterator - (BI)->begin) <= (BI)->size); \
    __quex_assert((BI)->bytes_left_n <= (BI)->size);    
#else
#   define QUEX_ASSERT_BUFFER_INFO(BI)      /* empty */
#   define QUEX_ASSERT_BUFFER_INFO_EASY(BI) /* empty */
#endif

#if ! defined (__QUEX_SETTING_PLAIN_C)
    extern "C" { 
#   include <iconv.h>
    }
#   include <quex/code_base/compatibility/iconv-argument-types.h>
#      include <cerrno>
#   else
#      include <errno.h>
#endif

#include <quex/code_base/temporary_macros_on>

#if ! defined (__QUEX_SETTING_PLAIN_C)
namespace quex {
#endif
    TEMPLATE_IN(InputHandleT) size_t __QuexBufferFiller_IConv_tell_character_index(QuexBufferFiller* alter_ego);
    TEMPLATE_IN(InputHandleT) void   __QuexBufferFiller_IConv_seek_character_index(QuexBufferFiller* alter_ego, 
                                                                                   const size_t      CharacterIndex); 
    TEMPLATE_IN(InputHandleT) size_t __QuexBufferFiller_IConv_read_characters(QuexBufferFiller*    alter_ego,
                                                                              QUEX_CHARACTER_TYPE* start_of_buffer, 
                                                                              const size_t         N);
    TEMPLATE_IN(InputHandleT) void   __QuexBufferFiller_IConv_destroy(QuexBufferFiller* alter_ego);

    QUEX_INLINE bool QuexBufferFiller_IConv_has_coding_dynamic_character_width(const char* Coding);

    QUEX_INLINE void QuexBufferFiller_IConv_BufferInfo_init(QuexBufferFiller_IConv_BufferInfo* me, 
                                                            uint8_t* Begin, size_t SizeInBytes);

    TEMPLATE_IN(InputHandleT) size_t __QuexBufferFiller_IConv_read_characters(QuexBufferFiller*    alter_ego,
                                                                              QUEX_CHARACTER_TYPE* user_memory_p, 
                                                                              const size_t         N);

    TEMPLATE_IN(InputHandleT) void __QuexBufferFiller_IConv_fill_raw_buffer(TEMPLATED(QuexBufferFiller_IConv)*);

    TEMPLATE_IN(InputHandleT) bool __QuexBufferFiller_IConv_convert(TEMPLATED(QuexBufferFiller_IConv)* me, 
                                                                    QuexBufferFiller_IConv_BufferInfo* drain);

    TEMPLATE_IN(InputHandleT) void __QuexBufferFiller_IConv_step_forward_n_characters(TEMPLATED(QuexBufferFiller_IConv)* me,
                                                       const size_t     ForwardN);

    TEMPLATE_IN(InputHandleT) void __QuexBufferFiller_IConv_mark_start_position(TEMPLATED(QuexBufferFiller_IConv)* me);

    TEMPLATE_IN(InputHandleT) void
    QuexBufferFiller_IConv_init(TEMPLATED(QuexBufferFiller_IConv)* me,
                                InputHandleT* input_handle, 
                                const char*   FromCoding,   const char* ToCoding,
                                size_t        RawBufferSize)
    { 
        const char* to_coding = ToCoding != 0x0 ? ToCoding : QUEX_SETTING_CORE_ENGINE_DEFAULT_CHARACTER_CODING;

        __quex_assert(RawBufferSize >= 6);  /* UTF-8 char can be 6 bytes long    */

        __QuexBufferFiller_init_functions(&me->base,
                                          TEMPLATED(__QuexBufferFiller_IConv_tell_character_index),
                                          TEMPLATED(__QuexBufferFiller_IConv_seek_character_index), 
                                          TEMPLATED(__QuexBufferFiller_IConv_read_characters),
                                          TEMPLATED(__QuexBufferFiller_IConv_destroy));

        /* Initialize the raw buffer that holds the plain bytes of the input file
         * (setup to trigger initial reload)                                                */
        me->ih = input_handle;

        /* Initialize the raw buffer that holds the plain bytes of the input file           */
        uint8_t* raw_buffer_p = MemoryManager_get_BufferFiller_RawBuffer(RawBufferSize);
        QuexBufferFiller_IConv_BufferInfo_init(&me->raw_buffer.base, raw_buffer_p, RawBufferSize);

        me->raw_buffer.base.bytes_left_n  = 0;  /* --> trigger reload                       */

        /* Initialize the conversion operations                                             */
        me->iconv_handle = iconv_open(to_coding, FromCoding);
        if( me->iconv_handle == (iconv_t)-1 ) {
            char tmp[128];
            snprintf(tmp, 127, "Conversion '%s' --> '%s' is not supported by 'iconv'.\n", FromCoding, to_coding);
            QUEX_ERROR_EXIT(tmp);
        }
        me->_constant_size_character_encoding_f = \
                        ! QuexBufferFiller_IConv_has_coding_dynamic_character_width(FromCoding);

        /* Setup the tell/seek of character positions                                       */
        __QuexBufferFiller_IConv_mark_start_position(me);
        me->raw_buffer.begin_stream_position = QUEX_INPUT_POLICY_TELL(me->ih, InputHandleT);
        me->raw_buffer.begin_character_index = 0;
        me->raw_buffer.end_stream_position       = me->raw_buffer.begin_stream_position;
        me->raw_buffer.iterators_character_index = 0;
        me->raw_buffer.fill_level_n            = 0;

        /*QUEX_UNIT_TEST_ICONV_INPUT_STRATEGY_PRINT_CONSTRUCTOR(FromCoding, ToCoding, me->iconv_handle);*/
        QUEX_ASSERT_BUFFER_INFO(&me->raw_buffer);
    }

    TEMPLATE_IN(InputHandleT) void   
    __QuexBufferFiller_IConv_destroy(QuexBufferFiller* alter_ego)
    { 
        TEMPLATED(QuexBufferFiller_IConv)* me = (TEMPLATED(QuexBufferFiller_IConv)*)alter_ego;
        QUEX_ASSERT_BUFFER_INFO(&me->raw_buffer);

        iconv_close(me->iconv_handle); 

        MemoryManager_free_BufferFiller_RawBuffer(me->raw_buffer.base.begin); 

        /* The memory manager allocated the space required for a buffer filler of this
         * type. Somewhere down the road it is known what size of memory belongs to this
         * pointer. */
        MemoryManager_free_BufferFiller(alter_ego);
    }

    TEMPLATE_IN(InputHandleT) size_t 
    __QuexBufferFiller_IConv_read_characters(QuexBufferFiller*    alter_ego,
                                             QUEX_CHARACTER_TYPE* user_memory_p, 
                                             const size_t         N)
    {
        __quex_assert(alter_ego != 0x0); 
        TEMPLATED(QuexBufferFiller_IConv)* me = (TEMPLATED(QuexBufferFiller_IConv)*)alter_ego;

        QuexBufferFiller_IConv_BufferInfo user_buffer;
        QuexBufferFiller_IConv_BufferInfo_init(&user_buffer, (uint8_t*)user_memory_p, 
                                               N * sizeof(QUEX_CHARACTER_TYPE));
        QUEX_ASSERT_BUFFER_INFO(&me->raw_buffer);

        /* TWO CASES:
         * (1) There are still some bytes in the raw buffer from the last load.
         *     in this case, first translate them and then maybe load the raw buffer
         *     again. (raw_buffer.base.bytes_left_n != 0)
         * (2) The raw buffer is empty. Then it must be loaded in order to get some
         *     basis for conversion. (raw_buffer.base.bytes_left_n == 0)                      */
        if( me->raw_buffer.base.bytes_left_n == 0 ) __QuexBufferFiller_IConv_fill_raw_buffer(me); 

        while( __QuexBufferFiller_IConv_convert(me, &user_buffer) == false )  
            __QuexBufferFiller_IConv_fill_raw_buffer(me); 

        if( user_buffer.bytes_left_n == 0 ) { 
            /* The buffer was filled to its limits. All 'N' characters have been written. */
            return N;
        } else { 
            /* The buffer was not filled completely, because the end of the file was 
             * reached. The fill level of the user buffer computes as:                    */
            const size_t ConvertedCharN = (user_buffer.size - user_buffer.bytes_left_n) / sizeof(QUEX_CHARACTER_TYPE);
#           ifdef QUEX_OPTION_ASSERTS
            /* '.bytes_left_n' is not always correct. '.iterator' points always directly 
              * behind the last written byte. */
            /* Cast to uint8_t to avoid that some smart guy provides a C++ overloading function */
            __QUEX_STD_memset((uint8_t*)(user_buffer.iterator), (uint8_t)0xFF, 
                              (user_buffer.begin + user_buffer.size) - user_buffer.iterator);
#           endif
            return ConvertedCharN;
        }
    }

    TEMPLATE_IN(InputHandleT) void 
    __QuexBufferFiller_IConv_fill_raw_buffer(TEMPLATED(QuexBufferFiller_IConv)*  me) 
    {
        /* Try to fill the raw buffer to its limits with data from the file.
         * The filling starts from its current position, thus the remaining bytes
         * to be translated are exactly the number of bytes in the buffer.              */
        QuexBufferFiller_IConv_BufferInfo* buffer = &me->raw_buffer.base;
        const size_t RemainingBytesN = buffer->bytes_left_n;
        QUEX_ASSERT_BUFFER_INFO(&me->raw_buffer);
        __quex_assert(me->raw_buffer.end_stream_position == QUEX_INPUT_POLICY_TELL(me->ih, InputHandleT));

        /* Update the character index / stream position infos */
        me->raw_buffer.begin_character_index = me->raw_buffer.iterators_character_index;
        me->raw_buffer.begin_stream_position =   me->raw_buffer.end_stream_position 
                                               - (STREAM_OFFSET_TYPE(InputHandleT))RemainingBytesN;

        /* There are cases (e.g. when a broken multibyte sequence occured at the end of 
         * the buffer) where there are bytes left in the raw buffer. These need to be
         * moved to the beginning of the buffer.                                        */
        if( buffer->iterator != buffer->begin ) {
            /* Be careful: Maybe one can use 'memcpy()' which is a bit faster but the
             * following is safe against overlaps.                                      */
            /* Cast to uint8_t to avoid a spurious function overload */
            __QUEX_STD_memmove((uint8_t*)(buffer->begin), (uint8_t*)(buffer->iterator), RemainingBytesN);
            /* Reset the position, so that new conversion get's the whole character.    */
            buffer->iterator = buffer->begin; 
        }

        uint8_t*     FillStartPosition = buffer->begin + RemainingBytesN;
        size_t       FillSize          = buffer->size  - RemainingBytesN;
        const size_t LoadedByteN = \
                     QUEX_INPUT_POLICY_LOAD_BYTES(me->ih, InputHandleT, FillStartPosition, FillSize);

        /* NOTE: In case of 'end of file' it is possible: bytes_left_n != buffer->size */
        me->raw_buffer.fill_level_n = LoadedByteN + RemainingBytesN;
        buffer->bytes_left_n        = LoadedByteN + RemainingBytesN; 

        me->raw_buffer.end_stream_position = QUEX_INPUT_POLICY_TELL(me->ih, InputHandleT);
        /* '.character_index' remains to be updated after character conversion */

        /*QUEX_UNIT_TEST_ICONV_INPUT_STRATEGY_PRINT_RAW_BUFFER_LOAD(LoadedByteN);*/
        QUEX_ASSERT_BUFFER_INFO(&me->raw_buffer);
    }

    TEMPLATE_IN(InputHandleT) bool 
    __QuexBufferFiller_IConv_convert(TEMPLATED(QuexBufferFiller_IConv)*                   me, 
                                     QuexBufferFiller_IConv_BufferInfo* drain) 
    {
        /* RETURNS:  true  --> User buffer is filled as much as possible with converted 
         *                     characters.
         *           false --> More raw bytes are needed to fill the user buffer.           
         *
         *  IF YOU GET A COMPILE ERROR HERE, THEN PLEASE HAVE A LOOK AT THE FILE:
         *
         *      quex/code_base/compatibility/iconv-argument-types.h
         * 
         *  The issue is, that 'iconv' is defined on different systems with different
         *  types of the second argument. There are two variants 'const char**'
         *  and 'char **'.  If you get an error here, consider defining 
         *
         *            -DQUEX_SETTING_ICONV_2ND_ARG_CONST_CHARPP
         *
         *  as a compile option. If you have an elegant solution to solve the problem for 
         *  plain 'C', then please, let me know <fschaef@users.sourceforge.net>.               */
        QuexBufferFiller_IConv_BufferInfo* source = &me->raw_buffer.base;
        const uint8_t*                     drain_start_iterator = drain->iterator;
        QUEX_ASSERT_BUFFER_INFO(&me->raw_buffer);
        QUEX_ASSERT_BUFFER_INFO_EASY(drain);
        size_t report = iconv(me->iconv_handle, 
                              __QUEX_ADAPTER_ICONV_2ND_ARG(&source->iterator), &source->bytes_left_n,
                              (char**)&(drain->iterator),                      &(drain->bytes_left_n));
        QUEX_ASSERT_BUFFER_INFO(&me->raw_buffer);

        /* The following only holds for fixed size **target** encodings. In Quex we assume that 
         * the buffer content on which the core engine operates has fixed character widths.    */   
        const size_t ConvertedCharN = (drain->iterator - drain_start_iterator)/sizeof(QUEX_CHARACTER_TYPE);
        me->raw_buffer.iterators_character_index += ConvertedCharN;

        if( report != (size_t)-1 ) { 
            __quex_assert(source->bytes_left_n == 0);
            /* The input sequence (raw buffer content) has been converted completely.
             * But, is the user buffer filled to its limits?                                   */
            if( drain->bytes_left_n == 0 ) {
                __quex_assert(drain->iterator == drain->begin + drain->size);
                return true; 
            }
            /* If the buffer was not filled completely, then was it because we reached EOF?
             * NOTE: Here, 'source->iterator' points to the position after the last byte
             *       that has been converted. If this is the end of the buffer, then it means
             *       that the raw buffer was read. If not, it means that the buffer has not been
             *       filled to its border which happens only if End of File occured.           */
            if( source->iterator != source->begin + source->size ) {
                __quex_assert(me->raw_buffer.fill_level_n != me->raw_buffer.base.size);
                return true;
            }
            else {
                /* Else: The user buffer is still hungry, thus the raw buffer needs more bytes. */
                /* source->bytes_left_n = 0 anyway, so 'refill' is triggered                    */
                return false; 
            }
        }

        switch( errno ) {
        default:
            QUEX_ERROR_EXIT("Unexpected setting of 'errno' after call to GNU's iconv().");

        case EILSEQ:
            QUEX_ERROR_EXIT("Invalid byte sequence encountered for given character coding.");

        case EINVAL:
            /* (*) Incomplete byte sequence for character conversion
             *     ('raw_buffer.base.iterator' points to the beginning of the incomplete sequence.)
             *     Please, refill the buffer (consider copying the bytes from raw_buffer.base.iterator 
             *     to the end of the buffer in front of the new buffer).                             */
            return false; 

        case E2BIG:
            /* (*) The input buffer was not able to hold the number of converted characters.
             *     (in other words we're filled up to the limit and that's what we actually wanted.) */
            return true;
        }
    }


    TEMPLATE_IN(InputHandleT) size_t 
    __QuexBufferFiller_IConv_tell_character_index(QuexBufferFiller* alter_ego)
    { 
        __quex_assert(alter_ego != 0x0); 
        TEMPLATED(QuexBufferFiller_IConv)* me = (TEMPLATED(QuexBufferFiller_IConv)*)alter_ego;
        /* The raw buffer iterator stands on the next character to be read. In general it holds
         * that the raw_buffer's iterator points to the first byte of the next character to be
         * converted when the next user buffer is to be filled.                                      */
        return me->raw_buffer.iterators_character_index; 
    }

    TEMPLATE_IN(InputHandleT) void   
    __QuexBufferFiller_IConv_seek_character_index(QuexBufferFiller* alter_ego, 
                                              const size_t          Index)
    { 
        /* NOTE: This differs from QuexBuffer_seek(...) in the sense, that it only sets the
         *       stream to a particular position given by a character index. QuexBuffer_seek(..)
         *       sets the _input_p to a particular position.                                      */
        __quex_assert(alter_ego != 0x0); 
        TEMPLATED(QuexBufferFiller_IConv)* me         = (TEMPLATED(QuexBufferFiller_IConv)*)alter_ego;
        const size_t     BeginIndex = me->raw_buffer.begin_character_index;

        /* Seek_character_index(Pos) means that the next time when a character buffer
         * is to be filled, this has to happen from position 'CharacterIndex'. 
         *
         * NOTE: The reference for character positioning is **not** directly the stream.
         * Moreover, it is the internal raw_buffer.position. It determines what characters 
         * are converted next into the user's buffer.                                             */
        if( Index == me->raw_buffer.iterators_character_index ) { 
            return;                                                /* Nothing to be done          */
        }
        /* Depending on the character encoding, the seek procedure can be optimized there are the 
         * following two cases:
         *
         *   (1) The coding uses **fixed** character widths (such as ASCII, UCS2, UCS4, etc.) where
         *       each character always occupies the same amount of bytes. Here, offsets can be 
         *       **computed**. This makes things faster.
         *   (2) The coding uses **dynamic** character width (e.g. UTF-8). Here the position cannot
         *       be computed. Instead, the conversion must start from a given 'known' position 
         *       until one reaches the specified character index.                                 */
        if( me->_constant_size_character_encoding_f ) { 
            /* (1) Fixed Character Width */
            const size_t EndIndex = BeginIndex + (me->raw_buffer.base.size / sizeof(QUEX_CHARACTER_TYPE));
            if( Index >= BeginIndex && Index < EndIndex ) {
                uint8_t* new_iterator = me->raw_buffer.base.begin + (Index - BeginIndex) * sizeof(QUEX_CHARACTER_TYPE);
                me->raw_buffer.base.bytes_left_n -= (new_iterator - me->raw_buffer.base.iterator);
                me->raw_buffer.base.iterator             = new_iterator;
                me->raw_buffer.iterators_character_index = Index;
            }
            else  /* Index not in [BeginIndex:EndIndex) */ {
                STREAM_POSITION_TYPE(InputHandleT) avoid_tmp_arg =
                    (STREAM_POSITION_TYPE(InputHandleT))(Index * sizeof(QUEX_CHARACTER_TYPE) + me->start_position);
                QUEX_INPUT_POLICY_SEEK(me->ih, InputHandleT, avoid_tmp_arg);
                me->raw_buffer.end_stream_position = avoid_tmp_arg;
                me->raw_buffer.base.bytes_left_n = 0;                                 /* Trigger reload */
                me->raw_buffer.base.iterator             = me->raw_buffer.base.begin;
                me->raw_buffer.begin_character_index     = Index;
                me->raw_buffer.iterators_character_index = Index;
            }
        } 
        else  { 
            /* (2) Dynamic Character Width */
            /* Setting the iterator to the begin of the raw_buffer initiates a conversion
             * start from this point.                                                              */
            if( Index == BeginIndex ) { 
                /* The 'read_characters()' function works on the content of the bytes
                 * in the raw_buffer. The only thing that has to happen is to reset 
                 * the raw buffer's position pointer to '0'.                                       */
                me->raw_buffer.base.bytes_left_n += (me->raw_buffer.base.iterator - me->raw_buffer.base.begin);
                me->raw_buffer.base.iterator             = me->raw_buffer.base.begin;
                me->raw_buffer.iterators_character_index = Index;
            }
            else if( Index > BeginIndex ) { 
                /* The searched index lies in the current raw_buffer or behind. Simply start 
                 * conversion from the current position until it is reached--but use the stuff 
                 * currently inside the buffer.                                                   */
                me->raw_buffer.base.bytes_left_n += (me->raw_buffer.base.iterator - me->raw_buffer.base.begin);
                me->raw_buffer.base.iterator             = me->raw_buffer.base.begin;
                me->raw_buffer.iterators_character_index = me->raw_buffer.begin_character_index;
                __QuexBufferFiller_IConv_step_forward_n_characters(me, Index - BeginIndex);
            }
            else  /* Index < BeginIndex */ {
                /* No idea where to start --> start from the beginning. In some cases this might
                 * mean a huge performance penalty. But note, that is only occurs at pre-conditions
                 * that are very very long and happen right at the beginning of the raw buffer.   */
                QUEX_INPUT_POLICY_SEEK(me->ih, InputHandleT, me->start_position);
                me->raw_buffer.end_stream_position = me->start_position;
                me->raw_buffer.base.bytes_left_n = 0; /* trigger reload, not only conversion           */
                me->raw_buffer.base.iterator             = me->raw_buffer.base.begin;
                me->raw_buffer.begin_character_index     = 0;
                me->raw_buffer.iterators_character_index = 0;
                me->raw_buffer.fill_level_n              = 0;
                __QuexBufferFiller_IConv_step_forward_n_characters(me, Index);
            } 
        }
        QUEX_ASSERT_BUFFER_INFO(&me->raw_buffer);
        __quex_assert(me->raw_buffer.iterators_character_index == Index);
    }

    TEMPLATE_IN(InputHandleT) void 
    __QuexBufferFiller_IConv_step_forward_n_characters(TEMPLATED(QuexBufferFiller_IConv)* me,
                                                       const size_t     ForwardN)
    { 
        /* We cannot use the raw buffer at this point in time, since this is required 
         * to interpret characters. Whenever a dynamic size coding is required this
         * way of searching may become necessary.                                     */
#       ifdef QUEX_OPTION_ASSERTS
        const size_t         TargetIndex = me->raw_buffer.iterators_character_index + ForwardN;
#       endif
        const size_t         ChunkSize = QUEX_SETTING_BUFFER_FILLER_SEEK_TEMP_BUFFER_SIZE;
        QUEX_CHARACTER_TYPE  chunk[QUEX_SETTING_BUFFER_FILLER_SEEK_TEMP_BUFFER_SIZE];
        /* This seek operation thought to support only cases where character positions
         * cannot be computed--as is the case for fixed character width encodings.    */
        __quex_assert(me->_constant_size_character_encoding_f == false);
        /* Same restriction as for raw_buffer (see above) */
        __quex_assert(QUEX_SETTING_BUFFER_FILLER_SEEK_TEMP_BUFFER_SIZE >= 6);
        QUEX_ASSERT_BUFFER_INFO(&me->raw_buffer);

        /* STRATEGY: Starting from a certain point in the file we read characters
         *           Convert one-by-one until we reach the given character index      */
        size_t  remaining_character_n = ForwardN;

        /* We are now at character index 'CharacterIndex - remaining_character_n' in the stream.
         * It remains to interpret 'remaining_character_n' number of characters. Since the
         * the interpretation is best done using a buffer, we do this in chunks.      */ 
        for(; remaining_character_n > ChunkSize; remaining_character_n -= ChunkSize )  
            me->base.read_characters(&me->base, (QUEX_CHARACTER_TYPE*)chunk, ChunkSize);
        if( remaining_character_n ) 
            me->base.read_characters(&me->base, (QUEX_CHARACTER_TYPE*)chunk, remaining_character_n);
       
        __quex_assert(me->raw_buffer.iterators_character_index == TargetIndex);
    }

    TEMPLATE_IN(InputHandleT) void 
    __QuexBufferFiller_IConv_mark_start_position(TEMPLATED(QuexBufferFiller_IConv)* me) 
    { 
       __quex_assert(me != 0x0); 
       me->start_position = QUEX_INPUT_POLICY_TELL(me->ih, InputHandleT);
    }

    TEMPLATE_IN(InputHandleT) void 
    __QuexBufferFiller_IConv_reset_start_position(TEMPLATED(QuexBufferFiller_IConv)* me) 
    {
        __quex_assert(me != 0x0); 
        QUEX_INPUT_POLICY_SEEK(me->ih, InputHandleT, me->start_position);
        me->raw_buffer.end_stream_position = me->start_position;
    }

    QUEX_INLINE bool 
    QuexBufferFiller_IConv_has_coding_dynamic_character_width(const char* Coding) 
    {
        return true; /* TODO: distinguish between different coding formats   */
        /*           //       'true' is safe, but possibly a little slower.  */
    }

    QUEX_INLINE void
    QuexBufferFiller_IConv_BufferInfo_init(QuexBufferFiller_IConv_BufferInfo* me, 
                                           uint8_t* Begin, size_t SizeInBytes)
    {
        me->begin        = Begin;
        me->size         = SizeInBytes;
        me->iterator     = me->begin;
        me->bytes_left_n = me->size;

#       ifdef QUEX_OPTION_ASSERTS
        /* Cast to uint8_t to avoid that some smart guy provides a C++ overloading function */
        __QUEX_STD_memset((uint8_t*)Begin, (uint8_t)0xFF, SizeInBytes);
#       endif
    }

#undef CLASS

#if ! defined(__QUEX_SETTING_PLAIN_C)
}
#endif

#include <quex/code_base/temporary_macros_off>

#include <quex/code_base/buffer/BufferFiller.i>

#endif /* __INCLUDE_GUARD__QUEX_BUFFER_INPUT_STRATEGY_ICONV__ */
