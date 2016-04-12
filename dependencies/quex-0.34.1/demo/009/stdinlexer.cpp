#include<fstream>    
#include<iostream> 
#include<sstream> 

#include <./tiny_lexer>

int 
main(int argc, char** argv) 
{        
    using namespace std;

    // (*) create token
    quex::Token         Token;
    // (*) create the lexical analyser
    //     if no command line argument is specified user file 'example.txt'
    quex::tiny_lexer*    qlex = new quex::tiny_lexer();

    cout << ",------------------------------------------------------------------------------------\n";
    cout << "| [START]\n";
    cout << "Please, type an arbitrary sequence of the following:\n";
    cout << "-- One of the words: 'hello', 'world', 'hallo', 'welt', 'bonjour', 'le monde'.\n";
    cout << "-- An integer number.\n";
    cout << "-- The word 'bye' in order to terminate.\n";
    cout << "Please, terminate each line with pressing [enter].\n";


    int number_of_tokens = 0;
    while( cin ) {
        // Read a line from standard input
        cin.getline((char*)qlex->buffer_begin(), qlex->buffer_size());
        cout << "[[Received " << cin.gcount() << " characters in line.]]\n";
        // Prepare the read the characters that we just flooded into the buffer
        qlex->buffer_prepare(cin.gcount());
        
        // Loop until the 'termination' token arrives
        do {
            qlex->get_token(&Token);
            cout << string(Token) << endl;
            ++number_of_tokens;
        } while( Token.type_id() != QUEX_TKN_TERMINATION && Token.type_id() != QUEX_TKN_BYE );
        
        cout << "[[End of Input]]\n";
        if( qlex->buffer_distance_to_text_end() != 0 ) {
            // The end of the text is always zero terminated, that is why we can print it
            cout << "[[Warning: not all characters treated.]]\n";
        }

        if( Token.type_id() == QUEX_TKN_BYE ) break;
    }

    cout << "| [END] number of token = " << number_of_tokens << "\n";
    cout << "`------------------------------------------------------------------------------------\n";

    delete qlex;

    return 0;
}

