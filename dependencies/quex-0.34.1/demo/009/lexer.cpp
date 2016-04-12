#include<fstream>    
#include<iostream> 
#include<sstream> 

// (*) include lexical analyser header
#include <./tiny_lexer>

using namespace std;

quex::tiny_lexer* get_file_input();
quex::tiny_lexer* get_stringstream_input();


int 
main(int argc, char** argv) 
{        
    // (*) create token
    quex::Token        Token;
    // (*) create the lexical analyser
    //     if no command line argument is specified user file 'example.txt'
    quex::tiny_lexer*   qlex = 0x0;

    if( argc < 2 ) {
        cout << "At least one command line argument required.\n";
        return -1;
    } else if ( strcmp(argv[1], "FILE") == 0 ) {
        qlex = get_file_input();
    } else if ( strcmp(argv[1], "stringstream") == 0 ) {
        qlex = get_stringstream_input();
    } else {
        cout << "Experiment " << argv[1] << " not supported by this application.\n";
        return -1;
    }

    cout << ",------------------------------------------------------------------------------------\n";
    cout << "| [START]\n";

    int number_of_tokens = 0;
    // (*) loop until the 'termination' token arrives
    do {
        // (*) get next token from the token stream
        qlex->get_token(&Token);

        // (*) print out token information
        //     -- name of the token
        cout << string(Token);
        cout << endl;

        ++number_of_tokens;

        // (*) check against 'termination'
    } while( Token.type_id() != QUEX_TKN_TERMINATION );

    cout << "| [END] number of token = " << number_of_tokens << "\n";
    cout << "`------------------------------------------------------------------------------------\n";

    delete qlex;

    return 0;
}

quex::tiny_lexer* 
get_file_input()
{
    /* Normal File Input */
    cout << "FILE* (stdio.h):\n";
    cout << "   Note this works only when engine is generated with -b 1 (or no -b)\n";
    cout << "   and therefore QUEX_CHARACTER_TYPE == uint8_t.\n";
    assert(sizeof(QUEX_CHARACTER_TYPE) == sizeof(uint8_t));
    return new quex::tiny_lexer("example.txt");
}

quex::tiny_lexer* 
get_stringstream_input()
{
    /* Normal String Stream Input */
    std::stringstream    my_stream;
    cout << "stringstream:\n";
    cout << "   Note this works only when engine is generated with -b 1 (or no -b)\n";
    cout << "   and therefore QUEX_CHARACTER_TYPE == uint8_t.\n";
    assert(sizeof(QUEX_CHARACTER_TYPE) == sizeof(uint8_t));

    my_stream << "bonjour le monde hello world hallo welt";

    return new quex::tiny_lexer(&my_stream);
}

