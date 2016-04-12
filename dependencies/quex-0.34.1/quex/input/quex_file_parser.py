#! /usr/bin/env python
# Quex is  free software;  you can  redistribute it and/or  modify it  under the
# terms  of the  GNU Lesser  General  Public License  as published  by the  Free
# Software Foundation;  either version 2.1 of  the License, or  (at your option)
# any later version.
# 
# This software is  distributed in the hope that it will  be useful, but WITHOUT
# ANY WARRANTY; without even the  implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the  GNU Lesser General Public License for more
# details.
# 
# You should have received a copy of the GNU Lesser General Public License along
# with this  library; if not,  write to the  Free Software Foundation,  Inc., 59
# Temple Place, Suite 330, Boston, MA 02111-1307 USA
#
################################################################################
#
from   quex.frs_py.file_in          import *
from   quex.token_id_maker          import TokenInfo
from   quex.exception               import RegularExpressionException
import quex.lexer_mode               as lexer_mode
import quex.input.mode_definition    as mode_definition
import quex.input.regular_expression as regular_expression
import quex.input.code_fragment      as code_fragment
from   quex.core_engine.generator.action_info import UserCodeFragment


def do(file_list, Setup):
    global mode_db

    for file in file_list:
        fh = open_file_or_die(file, Codec="utf-8")

        # read all modes until end of file
        try:
            while 1 + 1 == 2:
                parse_section(fh, Setup)
        except EndOfStreamException:
            pass
        except RegularExpressionException, x:
            error_msg(x.message, fh)
        
    return lexer_mode.mode_db

def __parse_domain_of_whitespace_separated_elements(fh, CodeFragmentName, ElementNames, MinElementN):   
    """Returns list of lists, where 
     
         record_list[i][k]  means element 'k' of line 'i'

       NOTE: record_list[i][-1] contains the line 'i' as it appeared as a whole.
             record_list[i][-2] contains the line number of line in the given file.

    """       
    start_line_n = get_current_line_info_number(fh)
    dummy, i = read_until_letter(fh, ["{"], Verbose=True)
    if i == -1: 
        error_msg("missing '{' after %s statement" % CodeFragmentName, fh)
    #
    line_n = start_line_n       
    record_list = []
    while 1 + 1 == 2:
        line = fh.readline()
        line_n += 1
        #
        if line == "": 
            error_msg("found end of file while parsing a '%s' range.\n" % CodeFragmentName + \
                      "range started here.", fh, start_line_n)    
        line = line.strip()
        if line == "":                           continue           # empty line
        elif line[0] == '}':                     return record_list # end of define range
        elif len(line) > 1 and line[:2] == "//": continue           # comment

        # -- interpret line as list of whitespace separated record elements
        fields = line.split()    
        if fields != [] and is_identifier(fields[0]) == False:
            error_msg("'%s' is not a valid identifier." % fields[0], fh)

        if len(fields) < MinElementN: 
            format_str = ""
            for element in ElementNames:
                format_str += "%s   " % element 
            error_msg("syntax error in definition list\n" + \
                      "format: %s  NEWLINE" % format_str , fh, line_n)
        record_list.append(fields + [line_n, line])    

    assert True == False, "this code section should have never been reached!"

def parse_section(fh, Setup):

    # NOTE: End of File is supposed to be reached when trying to read a new
    #       section. Thus, the end-of-file catcher does not encompass the beginning.
    position = fh.tell()
    skip_whitespace(fh)
    word = read_next_word(fh)

    try:
        # (*) determine what is defined
        #
        #     -- 'mode { ... }'   => define a mode
        #     -- 'start = ...;'   => define the name of the initial mode
        #     -- 'header { ... }' => define code that is to be pasted on top
        #                            of the engine (e.g. "#include<...>")
        #     -- 'body { ... }'   => define code that is to be pasted in the class' body
        #                            of the engine (e.g. "public: int  my_member;")
        #     -- 'init { ... }'   => define code that is to be pasted in the class' constructors
        #                            of the engine (e.g. "my_member = -1;")
        #     -- 'define { ... }' => define patterns shorthands such as IDENTIFIER for [a-z]+
        #     -- 'token { ... }'  => define token ids
        #
        if word == "start":
            parse_initial_mode_definition(fh)
            return
        
        elif word == "header":
            fragment = code_fragment.parse(fh, "header", AllowBriefTokenSenderF=False)        
            lexer_mode.header = fragment
            return

        elif word == "body":
            fragment = code_fragment.parse(fh, "body", AllowBriefTokenSenderF=False)        
            lexer_mode.class_body = fragment
            return

        elif word == "init":
            fragment = code_fragment.parse(fh, "init", AllowBriefTokenSenderF=False)
            lexer_mode.class_init = fragment
            return
            
        elif word == "define":
            parse_pattern_name_definitions(fh, Setup)
            return

        elif word == "token":       
            parse_token_id_definitions(fh, Setup)
            return

        elif word == "mode":
            mode_definition.parse(fh, Setup)
            return
        else:
            error_msg("sequence '%s' not recognized as valid keyword in this context\n" % word + \
                      "use: 'mode', 'header', 'body', 'init', 'define', 'token' or 'start'", fh)
    except EndOfStreamException:
        fh.seek(position)
        error_msg("End of file reached while parsing '%s' section" % word, fh)

def parse_pattern_name_definitions(fh, Setup):
    """Parses pattern definitions of the form:
   
          WHITESPACE  [ \t\n]
          IDENTIFIER  [a-zA-Z0-9]+
          OP_PLUS     "+"
          
       That means: 'name' whitespace 'regular expression' whitespace newline.
       Comments can only be '//' nothing else and they have to appear at the
       beginning of the line.
       
       One regular expression can have more than one name, but one name can 
       only have one regular expression.
    """
    # NOTE: Catching of EOF happens in caller: parse_section(...)

    def __closing_bracket(fh):
        position = fh.tell()
        dummy = fh.read(1)
        if dummy == "}": return True
        fh.seek(position)
        return False

    #
    dummy, i = read_until_letter(fh, ["{"], Verbose=True)

    while 1 + 1 == 2:
        skip_whitespace(fh)

        if __closing_bracket(fh): 
            return
        
        # -- get the name of the pattern
        skip_whitespace(fh)
        pattern_name = read_identifier(fh)
        if pattern_name == "":
            raise RegularExpressionException("Missing identifier for pattern definition.")

        skip_whitespace(fh)

        if __closing_bracket(fh): 
            raise RegularExpressionException("Missing regular expression for pattern definition '%s'." % \
                                             pattern_name)

        # -- parse regular expression, build state machine
        regular_expression_obj, state_machine = regular_expression.parse(fh, AllowNothingIsFineF=True)
        
        lexer_mode.shorthand_db[pattern_name] = \
                lexer_mode.PatternShorthand(pattern_name, state_machine, 
                                            fh.name, get_current_line_info_number(fh),
                                            regular_expression_obj)

def parse_token_id_definitions(fh, Setup):
    """Parses token definitions of the form:
  
          TOKEN_ID_NAME [Number] [TypeName] 
          
       For example:

          TKN_IDENTIFIER   1002  std::string
  
       defines an token_id with value 1002 and type std::string.
   
          TKN_NUMBER   std::double
          TKN_FLAG     1008
          TKN_NUMBER   2999       
         
       defines an id TKN_NUMBER, where the type is set to 'std::string'. Then
       TKN_FLAG is defined as numerical value 1008. Finally an addition to 
       TKN_NUMBER is made, saying that is has the numerical value of 2999.       
          
    """
    # NOTE: Catching of EOF happens in caller: parse_section(...)
    #
    record_list = __parse_domain_of_whitespace_separated_elements(fh, "define", 
                                                                  ["TOKEN-ID-NAME", 
                                                                   "[Number]", "[TypeName]"], 1)
    db = lexer_mode.token_id_db
    for record in record_list:
        # NOTE: record[-1] -> line text, record[-2] -> line number
        #       record[:-2] fields of the line
        line_n    = record[-2]
        name      = record[0]
        type_name = None
        number    = None
        #
        # -- check the name, if it starts with the token prefix paste a warning
        token_prefix = Setup.input_token_id_prefix
        if name.find(token_prefix) == 0:
            error_msg("Token identifier '%s' starts with token prefix '%s'.\n" % (name, token_prefix) + \
                      "Token prefix is mounted automatically. This token id appears in the source\n" + \
                      "code as '%s%s'." % (token_prefix, name), \
                      fh, DontExitF=True)

        #
        if len(record) - 2 > 1: 
            candidate = record[1]
            # does candidate consist only of digits ? --> number
            # is first character a letter or '_' ?    --> type_name     
            if candidate.isdigit():           number    = long(candidate)
            elif is_identifier(candidate[0]): type_name = candidate
        #
        if len(record) - 2 > 2:
            candidate = record[2]
            # is first character a letter or '_' ?      
            if is_identifier(candidate[0]): 
                if type_name != None:
                    error_msg("Token can only have *one* type associated with it", fh, line_n)
                type_name = candidate
        #
        if not db.has_key(name): 
            db[name] = TokenInfo(name, number, type_name, fh.name, line_n)
        else:
            if number != None:    db[name].id        = number
            if type_name != None: db[name].type_name = type_name
            db[name].positions.append([fh.name, line_n])

def parse_initial_mode_definition(fh):
    # NOTE: Catching of EOF happens in caller: parse_section(...)

    verify_next_word(fh, "=")
    # specify the name of the intial lexical analyser mode
    skip_whitespace(fh)
    mode_name = read_identifier(fh)
    verify_next_word(fh, ";", Comment="Since quex version 0.33.5 this is required.")

    if lexer_mode.initial_mode.get_code() != "":
        error_msg("start mode defined more than once!", fh, DontExitF=True)
        error_msg("previously defined here",
                  lexer_mode.initial_mode.filename,
                  lexer_mode.initial_mode.line_n)
        
    lexer_mode.initial_mode = UserCodeFragment(mode_name, fh.name, get_current_line_info_number(fh), None)

