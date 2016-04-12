#include<fstream>    
#include<iostream> 

// (*) include lexical analyser header
#include <./tiny_lexer>

using namespace std;

int 
main(int argc, char** argv) 
{        
    // (*) create token
    quex::Token        Token;
    // (*) create the lexical analyser
    //     if no command line argument is specified user file 'example.txt'
    quex::tiny_lexer*  qlex = new quex::tiny_lexer(argc == 1 ? "example.txt" : argv[1], "UTF-8");

    // (*) print the version 
    // cout << qlex->version() << endl << endl;

    cout << ",------------------------------------------------------------------------------------\n";
    cout << "| [START]\n";

    int number_of_tokens = 0;
    // (*) loop until the 'termination' token arrives
    do {
        // (*) get next token from the token stream
        qlex->get_token(&Token);

        // (*) print out token information
        //     -- line number and column number
        cout << "(" << qlex->line_number() << ", " << qlex->column_number() << ")  \t";
        //     -- name of the token
        cout << string(Token);
        cout << endl;

        ++number_of_tokens;

        // (*) check against 'termination'
    } while( Token.type_id() != QUEX_TKN_TERMINATION );

    cout << "| [END] number of token = " << number_of_tokens << "\n";
    cout << "`------------------------------------------------------------------------------------\n";

    return 0;
}
