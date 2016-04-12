#include<fstream>    
#include<iostream> 

// (*) include lexical analyser header
#include "c_lexer"

using namespace std;

double     benchmark(std::FILE*, const size_t FileSize, double* repetition_n);

double     overhead(std::FILE*, 
                   const int SimulatedFileSize, const size_t SimulatedTokenN, 
                   const double RepetitionN);

// report.c:
size_t    get_file_size(const char*, bool SilentF=false);
void      print_date_string();
size_t    count_token_n(std::FILE*);
double    report(clock_t StartTime, double RepetitionN, size_t FileSize, size_t CharacterSize);
void      final_report(double TimePerRun, double RefTimePerRun, const char* ThisExecutableName, const char* Filename, size_t FileSize, size_t TokenN, double RepetitionN);

int 
main(int argc, char** argv) 
{        
    std::FILE*  fh = 0x0;

    {
        if( argc != 2 ) { return -1; }

        fh = fopen(argv[1], "r");
        if( fh == NULL ) { 
            cerr << "File '" << argv[1] << "' not found.\n";
            return -1;
        }
    }
    const size_t TokenN = count_token_n(fh); 
    fseek(fh, 0, SEEK_SET);
    const size_t FileSize = get_file_size(argv[1]);
    fseek(fh, 0, SEEK_SET);
    double        repetition_n;
    const double  TimePerRun = benchmark(fh, FileSize, &repetition_n);
    fseek(fh, 0, SEEK_SET);
    /* Measure the overhead of the measurement */
    const double  RefTimePerRun = overhead(fh, FileSize, TokenN, repetition_n);

    final_report(TimePerRun, RefTimePerRun, argv[0], argv[1], FileSize, TokenN, repetition_n);
    return 0;
} 

#ifndef QUEX_BENCHMARK_SERIOUS
void __PRINT_START()
{
    cout << ",------------------------------------------------------------------------------------\n";
    cout << "| [START]\n";
}
void __PRINT_END()
{
    cout << "| [END] \n";
    cout << "`------------------------------------------------------------------------------------\n";
}
#else 
void __PRINT_START() { }
void __PRINT_END() { }
#endif

double
benchmark(std::FILE* fh, const size_t FileSize, double* repetition_n)
{
    using namespace std;
    //quex::token*   TokenP;
    int            token_id = QUEX_TKN_TERMINATION;
    //
    // -- repeat the experiment, so that it takes at least 20 seconds
    const clock_t  StartTime = clock();
#   ifdef QUEX_BENCHMARK_SERIOUS
    const clock_t  MinExperimentTime = 10 * CLOCKS_PER_SEC + StartTime;
#   else
    const clock_t  MinExperimentTime = StartTime;
#   endif
    int            checksum = 0;
    size_t         token_n = 0;
    int            checksum_ref = -1;
    //
    quex::c_lexer  qlex(fh);

    do { 
        checksum       = 777;
        *repetition_n += 1.0f;
        __PRINT_START(); /* No Operation if QUEX_BENCHMARK_SERIOUS is defined */
        
        do {  
            // qlex->get_token(&TokenP);
            token_id = qlex.get_token();

            // checksum = (checksum + TokenP->type_id()) % 0xFF; 
            checksum = (checksum + token_id) % 0xFF; 

#           if ! defined(QUEX_BENCHMARK_SERIOUS)
            cout << /*qlex.line_number() << ": " <<*/ quex::Token::map_id_to_name(token_id) << endl;
#           endif

            token_n += 1;
        // } while( TokenP->type_id() != QUEX_TKN_TERMINATION );
        } while( (unsigned)token_id != QUEX_TKN_TERMINATION );
        // Overhead-Intern: (addition, modulo division, assignment, increment by one, comparison) * token_n

        __PRINT_END();
        if( checksum_ref == -1 ) { 
            checksum_ref = checksum; 
        }
        else if( checksum != checksum_ref ) {
            cerr << "run:                " << *repetition_n << endl;
            cerr << "checksum-reference: " << checksum_ref << endl;
            cerr << "checksum:           " << checksum     << endl;
            throw std::runtime_error("Checksum failure");
        }
        qlex._reset();
    } while( clock() < MinExperimentTime );
    // Overhead:   Overhead-Intern
    //           + (assignment, increment by one, comparision * 2, _reset(),
    //              clock(), comparision) * RepetitionN
    
    cout << "//Benchmark (including overhead)\n";
    cout << "//    TokenN: " << (int)((token_n-1) / *repetition_n) << endl;
    return report(StartTime, *repetition_n, FileSize, /* CharacterSize = 1 */ 1);
}

double 
overhead(std::FILE* fh,
         const int SimulatedFileSize, const size_t SimulatedTokenN, const double RepetitionN)
{
    // This function is supposed to perform all 'frame' operations, but not the
    // analyzer function. This is to estimate the overhead implied by the test program.
    const clock_t StartTime = clock();
    double        repetition_n = 0.0;
    int           checksum = 0;
    //
    quex::c_lexer qlex(fh);

    checksum = 0; // here: prevent optimizing away of token_n --> out of loop
    while( repetition_n < RepetitionN ) { 
        repetition_n += 1.0f;
        
        size_t token_n = 0;
        do {
            checksum = (token_n + checksum) % 0xFF;  // use argument RepetitionN instead of a 
            //                                          // constant, so that it cannot be optimized away.
            token_n += 1;
        } while( token_n != SimulatedTokenN );
        // Overhead-Intern: (addition, modulo division, assignment, increment by one, comparison) * token_n
        
        checksum += token_n ^ clock();

        qlex._reset();
    }
    // Overhead:   Overhead-Intern
    //           + (assignment, increment by one, comparision * 2, _reset(),
    //              clock(), comparision) * RepetitionN
    
    // The 'Checksum' is printed for the sole purpose to prevent that the 
    // checksum computation is not optimized away. When the checksum computation is not
    // optimized away, then the token id reception cannot be optimized away.
    cout << "// Overhead:\n";
    cout << "//     Checksum (meaningless): " << checksum << " [1]" << std::endl;
    return report(StartTime, RepetitionN, SimulatedFileSize, /* CharacterSize [byte] */ 1);
}

