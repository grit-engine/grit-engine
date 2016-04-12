// -*- C++ -*- vim:set syntax=cpp:
#include <quex/code_base/buffer/Buffer.i>
namespace quex { 

inline
CLASS::CLASS()
: 
    // NOTE: dynamic_cast<>() would request derived class to be **defined**! 
    // Decision: "ease-of-use preceeds protection against a tremendous stupidity."
    self(*((__QUEX_SETTING_DERIVED_CLASS_NAME*)this)),
    __file_handle_allocated_by_constructor(0x0)
#   ifdef QUEX_OPTION_INCLUDE_STACK_SUPPORT
    , include_stack(this)
#   endif
#   ifdef QUEX_OPTION_STRING_ACCUMULATOR
    , accumulator(this)
#   endif
#   ifdef __QUEX_OPTION_INDENTATION_TRIGGER_SUPPORT
    , counter(this)
#   endif
{
    CLASS::__constructor_core<FILE>(0x0, QUEX_PLAIN, 0x0);
    QuexBuffer_setup_memory(&this->buffer, 
                            MemoryManager_get_BufferMemory(QUEX_SETTING_BUFFER_SIZE), QUEX_SETTING_BUFFER_SIZE);      
}

inline
CLASS::CLASS(const std::string&       Filename, 
             const char*              InputCodingName /* = 0x0 */, 
             QuexBufferFillerTypeEnum BFT /* = QUEX_AUTO */)
: 
    // NOTE: dynamic_cast<>() would request derived class to be **defined**! 
    // Decision: "ease-of-use preceeds protection against a tremendous stupidity."
    self(*((__QUEX_SETTING_DERIVED_CLASS_NAME*)this)),
    __file_handle_allocated_by_constructor(0x0)
#   ifdef QUEX_OPTION_INCLUDE_STACK_SUPPORT
    , include_stack(this)
#   endif
#   ifdef QUEX_OPTION_STRING_ACCUMULATOR
    , accumulator(this)
#   endif
#   ifdef __QUEX_OPTION_INDENTATION_TRIGGER_SUPPORT
    , counter(this)
#   endif
{
    // Buffer: Size = (see macro def.), Fallback = 10 Characters
    // prefer FILE* based buffers, because we can turn low-level buffering off.
    // ownership of FILE* id passed to the input strategy of the buffer
    std::FILE* fh = std::fopen(Filename.c_str(), "r");
    if( fh == NULL ) QUEX_ERROR_EXIT("Error on attempt to open specified file.");
    setbuf(fh, 0);   // turn off system based buffering!
    //               // this is essential to profit from the quex buffer!
    __constructor_core(fh, BFT, InputCodingName);
    // Recall, that this thing as to be deleted/closed
    __file_handle_allocated_by_constructor = fh;
}

inline
CLASS::CLASS(std::istream*            p_input_stream, 
             const char*              InputCodingName /* = 0x0 */,
             QuexBufferFillerTypeEnum BFT /* = QUEX_AUTO */)
:
    // NOTE: dynamic_cast<>() would request derived class to be **defined**! 
    // Decision: "ease-of-use preceeds protection against a tremendous stupidity."
    self(*((__QUEX_SETTING_DERIVED_CLASS_NAME*)this)),
    __file_handle_allocated_by_constructor(0x0)
#   ifdef QUEX_OPTION_INCLUDE_STACK_SUPPORT
    , include_stack(this)
#   endif
#   ifdef QUEX_OPTION_STRING_ACCUMULATOR
    , accumulator(this)
#   endif
#   ifdef __QUEX_OPTION_INDENTATION_TRIGGER_SUPPORT
    , counter(this)
#   endif
{
    if( p_input_stream == NULL ) QUEX_ERROR_EXIT("Error: received NULL as pointer to input stream.");
    __constructor_core(p_input_stream, BFT, InputCodingName);
}

inline
CLASS::CLASS(std::wistream*           p_input_stream, 
             const char*              InputCodingName /* = 0x0 */,
             QuexBufferFillerTypeEnum BFT /* = QUEX_AUTO */)
:
    // NOTE: dynamic_cast<>() would request derived class to be **defined**! 
    // Decision: "ease-of-use preceeds protection against a tremendous stupidity."
    self(*((__QUEX_SETTING_DERIVED_CLASS_NAME*)this)),
    __file_handle_allocated_by_constructor(0x0)
#   ifdef QUEX_OPTION_INCLUDE_STACK_SUPPORT
    , include_stack(this)
#   endif
#   ifdef QUEX_OPTION_STRING_ACCUMULATOR
    , accumulator(this)
#   endif
#   ifdef __QUEX_OPTION_INDENTATION_TRIGGER_SUPPORT
    , counter(this)
#   endif
{
    if( p_input_stream == NULL ) QUEX_ERROR_EXIT("Error: received NULL as pointer to input stream.");
    __constructor_core(p_input_stream, BFT, InputCodingName);
}


inline
CLASS::CLASS(std::FILE* fh, 
             const char*              InputCodingName /* = 0x0 */,
             QuexBufferFillerTypeEnum BFT /* = QUEX_AUTO */)
: 
    self(*((__QUEX_SETTING_DERIVED_CLASS_NAME*)this)),
    __file_handle_allocated_by_constructor(0x0)
#   ifdef QUEX_OPTION_INCLUDE_STACK_SUPPORT
    , include_stack(this)
#   endif
#   ifdef QUEX_OPTION_STRING_ACCUMULATOR
    , accumulator(this)
#   endif
#   ifdef __QUEX_OPTION_INDENTATION_TRIGGER_SUPPORT
    , counter(this)
#   endif
{
    if( fh == NULL ) QUEX_ERROR_EXIT("Error: received NULL as a file handle.");
    setbuf(fh, 0);   // turn off system based buffering!
    //               // this is essential to profit from the quex buffer!
    __constructor_core(fh, BFT, InputCodingName);
}


inline
CLASS::~CLASS() 
{
    QuexAnalyser_destruct(this);
#   ifdef QUEX_OPTION_TOKEN_SENDING_VIA_QUEUE 
    delete _token_queue;
#   endif
    if( __file_handle_allocated_by_constructor != 0x0 ) {
        std::fclose(__file_handle_allocated_by_constructor); 
    }
}

} // namespace quex
