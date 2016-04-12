// -*- C++ -*- vim: set syntax=cpp:
// (C) 2007 Frank-Rene Schaefer
// NO WARRRANTY
//
#ifndef __INCLUDE_GUARD_QUEX_BUFFER_ICONV_INPUT_STRATEGY_UNIT_TEST_HELPER__
#define __INCLUDE_GUARD_QUEX_BUFFER_ICONV_INPUT_STRATEGY_UNIT_TEST_HELPER__

#if ! defined (__QUEX_OPTION_UNIT_TEST_INPUT_STRATEGY_ICONV)

#   define  QUEX_UNIT_TEST_ICONV_INPUT_STRATEGY_PRINT_CONSTRUCTOR(X,Y,Z)     /* empty */
#   define  QUEX_UNIT_TEST_ICONV_INPUT_STRATEGY_PRINT_RAW_BUFFER_LOAD(X)     /* empty */
#   define  QUEX_UNIT_TEST_ICONV_INPUT_STRATEGY_PRINT_ICONV_REPORT(X)        /* empty */
#   define  QUEX_UNIT_TEST_ICONV_INPUT_STRATEGY_PRINT_RAW_AND_USER_BUFFER(X) /* empty */

#else 

namespace quex {

    QUEX_INLINE void 
    QuexBufferFiller_IConv_print_construction_info(TEMPLATED_CLASS* me,
                                                   const char* FromCoding, const char* ToCoding,
                                                   iconv_t IconvResult)
    {
        /* clear raw buffer, for easy visual control */
        for(int i=0; i<raw_buffer.size; ++i) {                
            raw_buffer.begin[i] = 0;                          
        }                                                     
        std::cout << "from: " << FromCoding << std::endl;     
        std::cout << "to:   " << ToCoding << std::endl;       

        if( IconvResult == (iconv_t)(-1) ) {                 
            switch( errno ) {                                 
            case EINVAL:  // incomplete sequence ecountered (cut in between character) 
                std::cout << "invalid conversion\n";      
            }                                                 
        }
    }

    QUEX_INLINE void
        CLASS::QUEX_UNIT_TEST_ICONV_INPUT_STRATEGY_PRINT_RAW_BUFFER_LOAD(size_t LoadedByteN)      
        {
            std::cout << "(*) loaded bytes = " << LoadedByteN << std::endl;  
            raw_buffer.print("raw buffer");
        }

    QUEX_INLINE void
        CLASS::QUEX_UNIT_TEST_ICONV_INPUT_STRATEGY_PRINT_ICONV_REPORT(size_t Report)                             
        {
            std::cout << "(*) converted\n";                                                     
            if( Report == (size_t)(-1) ) {
                switch( errno ) {                                                               
                case EILSEQ:  // invalid sequence encountered                               
                    std::cout << "invalid sequence\n";                                      
                    break;                                                                  
                case EINVAL:  // incomplete sequence ecountered (cut in between character)  
                    std::cout << "incomplete sequence\n";                                   
                    break;                                                                  
                case E2BIG:   // not enough room in output buffer                           
                    std::cout << "not enough room in output buffer\n";                      
                    break;                                                                  
                }                                                                               
            }                                                                                   
            else {                                                                              
                std::cout << "normal conversion completed!\n";                                  
            }
        }

    QUEX_INLINE void
        CLASS::QUEX_UNIT_TEST_ICONV_INPUT_STRATEGY_PRINT_RAW_AND_USER_BUFFER(CLASS::buffer_info* user_buffer)                  
        {
            raw_buffer.print("raw buffer");                                                  
            user_buffer->print("user buffer", user_buffer->position - user_buffer->begin);
        }

    QUEX_INLINE void 
        CLASS::buffer_info::print(const char* name, int until_idx /* = -1*/) 
        {
            if( until_idx == -1 ) until_idx = size;
            fprintf(stdout, "%s:\n", name);
            fprintf(stdout, "  position   = %i\n", (int)(position - begin));
            fprintf(stdout, "  bytes left = %i\n", bytes_left_n);
            fprintf(stdout, "  content = {\n");

            fprintf(stdout, "    ");
            for(int i=0; i < until_idx ; ++i) {
                unsigned char b = begin[i+0];
                fprintf(stdout, "%02X.", (unsigned)b);
                if( (i+1) % 10 == 0 ) fprintf(stdout, "\n    ");
            }
            fprintf(stdout, "\n");

            fprintf(stdout, "  }\n");
        }
}
#undef CLASS
#undef QUEX_INLINE

#endif

#endif // __INCLUDE_GUARD_QUEX_BUFFER_ICONV_INPUT_STRATEGY_UNIT_TEST_HELPER__
