#include <ctime>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <c_lexer>

using namespace std;

size_t    get_file_size(const char*, bool SilentF=false);
void      print_date_string();
size_t    count_token_n(std::FILE*);
double     report(clock_t StartTime, double RepetitionN, size_t FileSize, size_t CharacterSize);
void      final_report(double TimePerRun, double RefTimePerRun, const char* ThisExecutableName, const char* Filename, 
                       size_t FileSize, size_t TokenN, double RepetitionN);

void 
final_report(double TimePerRun, double RefTimePerRun, const char* ThisExecutableName, const char* FileName, size_t FileSize, size_t TokenN, double RepetitionN)
{
    const double  CharN          = (double)(FileSize) / (CHARACTER_SIZE);
    const double  CycleTime      = 1.0 / double(CPU_FREQ_MHZ) * 1e-6;
    //
    const double  TimePerChar    = TimePerRun  / CharN;
    const double  CCC            = TimePerChar / CycleTime;
    const double  RefTimePerChar = RefTimePerRun  / CharN;
    const double  RefCCC         = RefTimePerChar / CycleTime;

    double  TimePerToken = 0;
    double  CCT          = 0;
    double  RefTimePerToken = 0;
    double  RefCCT          = 0;

    if( TokenN == 1 ) { 
        TimePerToken    = TimePerRun;
        RefTimePerToken = RefTimePerRun;
    } else { 
        TimePerToken    = TimePerRun     / double(TokenN);
        RefTimePerToken = RefTimePerRun  / double(TokenN);
    }
    CCT    = TimePerToken / CycleTime;
    RefCCT = RefTimePerToken / CycleTime;

    cout << "//Result:\n";
    cout << "//   Time / Run:          " << (TimePerRun - RefTimePerRun)   << endl;
    cout << "//   Time / Char:         " << (TimePerChar - RefTimePerChar) << endl;
    cout << "//   Clock Cycles / Char: " << (CCC - RefCCC)                 << endl;
    cout << "{" << endl;
    cout << "   quex_version    = {" << QUEX_VERSION << "}, " << endl;
    cout << "   cpu_name        = {" << CPU_NAME << "}, " << endl;
    cout << "   cpu_code        = {" << CPU_CODE << "}, " << endl;
    cout << "   cpu_freq_mhz    = {" << CPU_FREQ_MHZ << "}, " << endl;
    cout << "   cc_name         = {" << CC_NAME << "}, " << endl;
    cout << "   cc_version      = {" << CC_VERSION << "}, " << endl;
    cout << "   cc_opt_flags    = {" << CC_OPTIMIZATION_FLAGS << "}, " << endl;
    cout << "   executable_size = {" << get_file_size(ThisExecutableName, true) << "}, " << endl;
    cout << "   os_name         = {" << OS_NAME << "}, " << endl;
    cout << "   tester_email    = {" << EMAIL << "}, " << endl;
    print_date_string();
    cout << "   file_name    = {" << FileName << "}, " << endl;
    cout << "   file_size    = {" << FileSize << "}, " << endl;
    cout << "   char_size    = {" << CHARACTER_SIZE << "}, " << endl;
    cout << "   buffer_size  = {" << QUEX_SETTING_BUFFER_SIZE << "}, " << endl;
#       ifdef QUEX_OPTION_LINE_NUMBER_COUNTING
    cout << "   line_count   = {true}," << endl;
#       else
    cout << "   line_count   = {false}," << endl;
#       endif
#       ifdef QUEX_OPTION_COLUMN_NUMBER_COUNTING
    cout << "   column_count = {true}," << endl;
#       else
    cout << "   column_count = {false}," << endl;
#       endif
    cout << "   note         = {" << NOTE << "}, " << endl;
    // Result
    cout << "   repetition_n               = {" << (unsigned int)(RepetitionN) << "}, " << endl;
    cout << "   time_per_repetition        = {" << (TimePerRun - RefTimePerRun) << "}," << endl;
    cout << "   token_n                    = {" << TokenN << "}, " << endl;
    cout << "   clock_cycles_per_character = {" << (CCC - RefCCC) << "}, " << endl;
    cout << "   clock_cycles_per_token     = {" << (CCT - RefCCT) << "}, " << endl;
    cout << "}\n" << endl;
}


double
report(clock_t StartTime, double RepetitionN, size_t FileSize, size_t CharacterSize)
{ 
    using namespace std;

    const clock_t EndTime    = clock();
    const double   TimeDiff   = (double)(EndTime - StartTime) / (double)CLOCKS_PER_SEC;
    const double   TimePerRun = TimeDiff / RepetitionN;

    cout << "//    Total Time:  " << TimeDiff          << " [sec]" << endl;
    cout << "//    Runs:        " << (long)RepetitionN << " [1]"   << endl;
    cout << "//    TimePerRun:  " << TimePerRun        << " [sec]" << endl;

    const double  CharN          = FileSize / CHARACTER_SIZE;
    const double  CycleTime      = 1.0 / (CPU_FREQ_MHZ * 1e6);
    const double  TimePerChar    = TimePerRun  / CharN;
    const double  CCC            = TimePerChar / CycleTime;

    cout << "//    Time / Char:         " << TimePerChar << endl;
    cout << "//    Clock Cycles / Char: " << CCC         << endl;

    return TimePerRun;
}

size_t
count_token_n(std::FILE* fh)
{
    using namespace std;
    quex::c_lexer  qlex(fh);
    //quex::token*    TokenP;
    int token_id = QUEX_TKN_TERMINATION;
    int token_n = 0;

    // (*) loop until the 'termination' token arrives
    for(token_n=0; ; ++token_n) {
        token_id = qlex.get_token();
        if( token_id == QUEX_TKN_TERMINATION ) break;
    } 
    cout << "// TokenN: " << token_n << " [1]"   << endl;
    return token_n;
}

size_t
get_file_size(const char* Filename, bool SilentF /*=false*/)
{
    using namespace std;
    struct stat s;
    stat(Filename, &s);
    if( ! SilentF ) {
        cout << "// FileSize: " << s.st_size << " [Byte] = "; 
        cout << double(s.st_size) / double(1024.0) << " [kB] = ";
        cout << double(s.st_size) / double(1024.0*1024.0) << " [MB]." << endl;
    }
    return s.st_size;
}

void
print_date_string()
{

    std::time_t  current_time     = time(NULL); 
    struct tm*   broken_down_time = std::gmtime(&current_time);
    
    std::cout << "   year         = {" << broken_down_time->tm_year + 1900   << "}," << endl;
    std::cout << "   month        = {" << broken_down_time->tm_mon  + 1    << "}," << endl;
    std::cout << "   day          = {" << broken_down_time->tm_mday        << "}," << endl;
}
