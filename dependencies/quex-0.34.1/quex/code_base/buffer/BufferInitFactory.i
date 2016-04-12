/* -*- C++ -*- vim: set syntax=cpp: */

#   ifndef __QUEX_SETTING_PLAIN_C
#   define BUFFER_FILLER_CONVERTER QuexBufferFiller_IConv<InputHandleT>
#   else
#   define BUFFER_FILLER_CONVERTER QuexBufferFiller_IConv
#   endif

QuexBufferFiller*
MemoryManager_get_BufferMemory(const size_t ByteN)
{
    return __QUEX_ALLOCATE_MEMORY(1, ByteN);
}

QuexBufferFiller*
MemoryManager_get_BufferFiller(QuexInputCodingTypeEnum)
{
    return __QUEX_ALLOCATE_MEMORY(1, QuexBufferFiller_get_memory_size(ICT);
}

QuexBufferFiller*
MemoryManager_get_BufferFiller_RawBuffer(const size_t ByteN)
{
    return __QUEX_ALLOCATE_MEMORY(1, ByteN);
}

QuexBufferFiller*
MemoryManager_free_BufferMemory(uint8_t* memory_chunk)
{
    __QUEX_FREE_ARRAY_MEMORY(memory_chunk);
}

QuexBufferFiller*
MemoryManager_free_BufferFiller(QuexBufferFiller* memory, QuexInputCodingTypeEnum ICT)
{
    __QUEX_FREE_MEMORY(memory);
}

QuexBufferFiller*
MemoryManager_free_BufferFiller_RawBuffer(QuexInputCodingTypeEnum)
{
    __QUEX_FREE_ARRAY_MEMORY(memory_chunk);
}


