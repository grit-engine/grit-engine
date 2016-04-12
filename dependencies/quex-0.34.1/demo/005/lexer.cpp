#include<fstream>    
#include<iostream> 

// (*) include lexical analyser header
#include <./tiny_lexer>
#include <./tiny_lexer-token_ids>

using namespace std;

int 
main(int argc, char** argv) 
{        
    // (*) create token
    quex::Token        Token;
    // (*) create the lexical analyser
    //     if no command line argument is specified user file 'example.txt'
    quex::tiny_lexer*  qlex = new quex::tiny_lexer(argc == 1 ? "example.txt" : argv[1]);

    // (*) print the version 
    // cout << qlex->version() << endl << endl;

    cout << ",------------------------------------------------------------------------------------\n";
    cout << "| [START]\n";

    int  number_of_tokens = 0;
    bool continue_lexing_f = true;
    // (*) loop until the 'termination' token arrives
    do {
        // (*) get next token from the token stream
        qlex->get_token(&Token);

        // (*) print out token information
        //     -- name of the token
        cout << Token.type_id_name() << "\t" << Token.text().c_str() << endl;

        switch( Token.type_id() ) {
        default: 
            break;

        case QUEX_TKN_INCLUDE: 
            {
                qlex->get_token(&Token);
                cout << Token.type_id_name() << "\t" << Token.text().c_str() << endl;
                if( Token.type_id() != QUEX_TKN_IDENTIFIER ) {
                    cout << "found 'include' without a subsequent filename. hm?\n";
                    break;
                }
               
                cout << ">> including: " << Token.text().c_str() << endl;
                FILE* fh = fopen((const char*)(Token.text().c_str()), "r");
                if( fh == 0x0 ) {
                    cout << "file not found\n";
                    return 0;
                }
                qlex->include_stack.push(fh);
                break;
                
            }
        case QUEX_TKN_TERMINATION:
            if( qlex->include_stack.pop() == false ) continue_lexing_f = false;
            else                                     cout << "<< return from include\n";
            break;
        }


        ++number_of_tokens;

        // (*) check against 'termination'
    } while( continue_lexing_f );

    cout << "| [END] number of token = " << number_of_tokens << "\n";
    cout << "`------------------------------------------------------------------------------------\n";

    return 0;
}
