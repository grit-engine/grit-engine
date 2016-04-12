#include<fstream>    
#include<iostream> 
#include<sstream> 

#include <./tiny_wlexer>

using namespace std;

quex::tiny_wlexer*   get_wstringstream_input();


int 
main(int argc, char** argv) 
{        
    // (*) create token
    quex::Token         Token;
    // (*) create the lexical analyser
    //     if no command line argument is specified user file 'example.txt'
    quex::tiny_wlexer*  qlex = 0x0;

    if( argc < 2 ) {
        cout << "At least one command line argument required.\n";
        return -1;
    } else if ( strcmp(argv[1], "wstringstream") == 0 ) {
        qlex = get_wstringstream_input();
    } else {
        cout << "Experiment " << argv[1] << " not supported by this application.\n";
        return -1;
    }

    cout << ",------------------------------------------------------------------------------------\n";
    cout << "| [START]\n";

    int number_of_tokens = 0;
    // (*) loop until the 'termination' token arrives
    do {
        qlex->get_token(&Token);
        cout << string(Token) << endl;
        ++number_of_tokens;
    } while( Token.type_id() != QUEX_TKN_TERMINATION );

    cout << "| [END] number of token = " << number_of_tokens << "\n";
    cout << "`------------------------------------------------------------------------------------\n";

    delete qlex;

    return 0;
}

quex::tiny_wlexer* 
get_wstringstream_input()
{
    /* Wide String Stream Input */
    std::wstringstream    my_stream;
    cout << "wstringstream:\n";
    cout << "   Note this works only when engine is generated with -b wchar_t\n";
    cout << "   and therefore QUEX_CHARACTER_TYPE == wchar_t.\n";

    assert(sizeof(QUEX_CHARACTER_TYPE) == sizeof(wchar_t));

    my_stream << L"bonjour le monde hello world hallo welt";

    return new quex::tiny_wlexer(&my_stream);
}
