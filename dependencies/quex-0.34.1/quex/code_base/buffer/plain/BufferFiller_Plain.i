/*  -*- C++ -*-  vim: set syntax=cpp: */
/* (C) 2008 Frank-Rene Schaefer*/
#ifndef __INCLUDE_GUARD__QUEX_BUFFER__BUFFER_FILLER_PLAIN_I__
#define __INCLUDE_GUARD__QUEX_BUFFER__BUFFER_FILLER_PLAIN_I__
/**/

#if ! defined (__QUEX_SETTING_PLAIN_C)
#   include <iostream> 
#   include <cerrno>
#   include <stdexcept>
#endif
#include <quex/code_base/definitions>
#include <quex/code_base/buffer/InputPolicy>
#include <quex/code_base/buffer/Buffer>
#include <quex/code_base/buffer/BufferFiller>
#include <quex/code_base/MemoryManager>

#include <quex/code_base/temporary_macros_on>

#if ! defined (__QUEX_SETTING_PLAIN_C)
namespace quex {
#endif

#   ifndef __QUEX_SETTING_PLAIN_C
#   define TEMPLATED(CLASS)   CLASS<InputHandleT>
#   else
#   define TEMPLATED(CLASS)   CLASS
#   endif

    TEMPLATE_IN(InputHandleT) size_t __BufferFiller_Plain_tell_character_index(QuexBufferFiller* alter_ego);
    TEMPLATE_IN(InputHandleT) void   __BufferFiller_Plain_seek_character_index(QuexBufferFiller* alter_ego, 
                                                                               const size_t      CharacterIndex); 
    TEMPLATE_IN(InputHandleT) size_t __BufferFiller_Plain_read_characters(QuexBufferFiller*    alter_ego,
                                                                          QUEX_CHARACTER_TYPE* start_of_buffer, 
                                                                          const size_t         N);
    TEMPLATE_IN(InputHandleT) void   __BufferFiller_Plain_destroy(QuexBufferFiller* alter_ego);

    TEMPLATE_IN(InputHandleT) void
    QuexBufferFiller_Plain_init(TEMPLATED(QuexBufferFiller_Plain)* me, 
                                InputHandleT*    input_handle)
    {
        __quex_assert(me != 0x0);
        __quex_assert(input_handle != 0x0);

        __QuexBufferFiller_init_functions(&me->base,
                                          TEMPLATED(__BufferFiller_Plain_tell_character_index),
                                          TEMPLATED(__BufferFiller_Plain_seek_character_index), 
                                          TEMPLATED(__BufferFiller_Plain_read_characters),
                                          TEMPLATED(__BufferFiller_Plain_destroy));
        /**/
        me->ih             = input_handle;
        me->start_position = QUEX_INPUT_POLICY_TELL(me->ih, InputHandleT);
    }

    TEMPLATE_IN(InputHandleT) void 
    __BufferFiller_Plain_destroy(QuexBufferFiller* alter_ego) 
    {
        /* The memory manager allocated the space required for a buffer filler of this
         * type. Somewhere down the road it is known what size of memory belongs to this
         * pointer. */
        MemoryManager_free_BufferFiller(alter_ego);

    }

    TEMPLATE_IN(InputHandleT) size_t 
    __BufferFiller_Plain_tell_character_index(QuexBufferFiller* alter_ego) 
    { 
       __quex_assert(alter_ego != 0x0); 
       /* The type cast is necessary, since the function signature needs to 
        * work with the first argument being of base class type. */
       TEMPLATED(QuexBufferFiller_Plain)* me = (TEMPLATED(QuexBufferFiller_Plain)*)alter_ego;

        __quex_assert(me->ih != 0x0); 
       return (QUEX_INPUT_POLICY_TELL(me->ih, InputHandleT) - me->start_position) / sizeof(QUEX_CHARACTER_TYPE); 
    }

    TEMPLATE_IN(InputHandleT) void 
    __BufferFiller_Plain_seek_character_index(QuexBufferFiller* alter_ego, const size_t CharacterIndex) 
    { 
        /* NOTE: This differs from QuexBuffer_seek(...) in the sense, that it only sets the
         *       stream to a particular position given by a character index. QuexBuffer_seek(..)
         *       sets the _input_p to a particular position.                                      */
        __quex_assert(alter_ego != 0x0); 
        /* The type cast is necessary, since the function signature needs to 
         * work with the first argument being of base class type. */
        TEMPLATED(QuexBufferFiller_Plain)* me = (TEMPLATED(QuexBufferFiller_Plain)*)alter_ego;
        __quex_assert(me->ih != 0x0); 

        long avoid_tmp_arg = (long)(CharacterIndex * sizeof(QUEX_CHARACTER_TYPE) + me->start_position); 
        QUEX_INPUT_POLICY_SEEK(me->ih, InputHandleT, avoid_tmp_arg);
    }

    TEMPLATE_IN(InputHandleT) size_t   
    __BufferFiller_Plain_read_characters(QuexBufferFiller*   alter_ego,
                                         QUEX_CHARACTER_TYPE* buffer_memory, const size_t N)  
    { 
        __quex_assert(alter_ego != 0x0); 
        __quex_assert(buffer_memory != 0x0); 
        /* The type cast is necessary, since the function signature needs to 
         * work with the first argument being of base class type. */
        TEMPLATED(QuexBufferFiller_Plain)* me = (TEMPLATED(QuexBufferFiller_Plain)*)alter_ego;

        __quex_assert(me->ih != 0x0); 
        const size_t ByteN = QUEX_INPUT_POLICY_LOAD_BYTES(me->ih, InputHandleT, 
                                                          buffer_memory, N * sizeof(QUEX_CHARACTER_TYPE));

        if( ByteN % sizeof(QUEX_CHARACTER_TYPE) != 0 ) {
            QUEX_ERROR_EXIT(
                "Error: Plain character encoding: End of file cuts in the middle a multi-byte character.");
        }
        return ByteN / sizeof(QUEX_CHARACTER_TYPE); 
    }

    TEMPLATE_IN(InputHandleT) void 
    __BufferFiller_Plain_mark_start_position(TEMPLATED(QuexBufferFiller_Plain)* me) 
    { 
       __quex_assert(me != 0x0); 
       me->start_position = QUEX_INPUT_POLICY_TELL(me->ih, InputHandleT);
    }

    TEMPLATE_IN(InputHandleT) void 
    __BufferFiller_Plain_reset_start_position(TEMPLATED(QuexBufferFiller_Plain)* me) 
    {
        __quex_assert(me != 0x0); 
        QUEX_INPUT_POLICY_SEEK(me->ih, InputHandleT, me->start_position);
    }

#   undef TEMPLATED_CLASS

#if ! defined (__QUEX_SETTING_PLAIN_C)
}  /* namespace quex*/
#endif

#include <quex/code_base/temporary_macros_off>

#include <quex/code_base/buffer/BufferFiller.i>
#endif /* __INCLUDE_GUARD__QUEX_BUFFER_INPUT_STRATEGY_PLAIN_I__ */
