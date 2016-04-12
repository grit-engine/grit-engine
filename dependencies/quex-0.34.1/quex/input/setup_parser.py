from   quex.DEFINITIONS import *
from   copy import copy
import sys
import os

from   quex.GetPot                 import GetPot
from   quex.frs_py.file_in         import open_file_or_die, error_msg, is_identifier, \
                                          extract_identifiers_with_specific_prefix, \
                                          delete_comment
import quex.lexer_mode  as lexer_mode
import quex.input.query as query
from   quex.token_id_maker import parse_token_id_file

from   quex.input.setup import setup, SETUP_INFO, LIST, FLAG, DEPRECATED

def do(argv):
    global setup

    # (*) Interpret Command Line (A) _____________________________________________________
    command_line = GetPot(argv)

    if command_line.search("--version", "-v"):
        print "Quex - A Mode Oriented Lexical Analyser"
        print "Version " + QUEX_VERSION
        print "(C) 2006-2008 Frank-Rene Schaefer"
        sys.exit(0)

    if command_line.search("--help", "-h"):
        print "Quex - A Mode Oriented Lexical Analyser"
        print "Please, consult the quex documentation for further help, or"
        print "visit http://quex.sourceforge.net."
        print "(C) 2006-2008 Frank-Rene Schaefer"
        sys.exit(0)

    for variable_name, info in SETUP_INFO.items():
        if info[1]   == LIST:
            the_list = command_line.nominus_followers(info[0])
            if setup.__dict__.has_key(variable_name):
                setup.__dict__[variable_name].extend(the_list)        
            else:
                setup.__dict__[variable_name] = the_list
        elif info[1] == FLAG:
            setup.__dict__[variable_name] = command_line.search(info[0])        
        else:
            setup.__dict__[variable_name] = command_line.follow(info[1], info[0])

    setup.QUEX_VERSION          = QUEX_VERSION
    setup.QUEX_INSTALLATION_DIR = QUEX_INSTALLATION_DIR
    setup.QUEX_TEMPLATE_DB_DIR  = QUEX_TEMPLATE_DB_DIR
            
    # (*) Output files
    setup.output_file_stem        = __prepare_file_name(setup, "")
    setup.output_token_id_file    = __prepare_file_name(setup, "-token_ids")
    setup.output_header_file      = __prepare_file_name(setup, "-internal.h")
    setup.output_code_file        = __prepare_file_name(setup, ".cpp")
    setup.output_core_engine_file = __prepare_file_name(setup, "-core-engine.cpp")

    setup.buffer_limit_code    = __get_integer(setup.buffer_limit_code, "--buffer-limit")
    setup.control_character_code_list = [setup.buffer_limit_code]

    setup.input_token_counter_offset = __get_integer(setup.input_token_counter_offset,
                                                     "--token-offset")
    setup.token_id_termination       = __get_integer(setup.token_id_termination, 
                                                     "--token-id-termination")
    setup.token_id_uninitialized     = __get_integer(setup.token_id_uninitialized, 
                                                     "--token-id-uninitialized")
    validate(setup, command_line, argv)

    if setup.input_foreign_token_id_file != "": 
        CommentDelimiterList = [["//", "\n"], ["/*", "*/"]]
        # Regular expression to find '#include <something>' and extract the 'something'
        # in a 'group'. Note that '(' ')' cause the storage of parts of the match.
        IncludeRE            = "#[ \t]*include[ \t]*[\"<]([^\">]+)[\">]"
        #
        parse_token_id_file(setup.input_foreign_token_id_file, setup.input_token_id_prefix, 
                            CommentDelimiterList, IncludeRE)

    # (*) Default values
    #     (Please, do not change this, otherwise no 'empty' options can be detected.)
    if setup.input_token_class_file == "": 
        setup.input_token_class_file = SETUP_INFO["input_token_class_file"][2]
    if setup.input_token_class_name == "": 
        setup.input_token_class_name = SETUP_INFO["input_token_class_name"][2]

    # (*) return setup ___________________________________________________________________
    return

def validate(setup, command_line, argv):
    """Does a consistency check for setup and the command line.
    """
    setup.output_directory = os.path.normpath(setup.output_directory)
    if setup.output_directory != "":
        # Check, if the output directory exists
        if os.access(setup.output_directory, os.F_OK) == False:
            error_msg("The directory %s was specified for output, but does not exists." % setup.output_directory)
        if os.access(setup.output_directory, os.W_OK) == False:
            error_msg("The directory %s was specified for output, but is not writeable." % setup.output_directory)

    # if the mode is 'plotting', then check wether a graphic format is speicified
    for plot_option in SETUP_INFO["plot_graphic_format"][0]:
        if plot_option in argv and setup.plot_graphic_format == "":
            error_msg("Option '%s' must be followed by a graphic format specifier (bmp, svg, jpg, ...)" % \
                      plot_option)

    # ensure that options are not specified twice
    for parameter, info in SETUP_INFO.items():
        occurence_n = 0 
        for option in info[0]:
            occurence_n += argv.count(option)
        if occurence_n > 1:
            error_msg("Received more than one of the following options:\n" + \
                      "%s" % repr(info[0])[1:-1])

    # (*) Check for 'Depraceted' Options ___________________________________________________
    for name, info in DEPRECATED.items():
        command_line_options = SETUP_INFO[name][0]
        comment                   = info[0]
        depreciated_since_version = info[1]
        for option in command_line_options:
            if command_line.search(option):
                error_msg("Command line option '%s' is ignored.\n" % option + \
                          "Last version of Quex supporting this option is version %s. Please, visit\n" % depreciated_since_version + \
                          "http://quex.sourceforge.net for download---Or use a more advanced approach.\n" + \
                          comment)
                          

    # (*) Check for 'Straying' Options ___________________________________________________
    options = []
    for key, info in SETUP_INFO.items():
        if key in DEPRECATED: continue
        if info[1] != None: options.extend(info[0])
    options.sort(lambda a,b: cmp(a.replace("-",""), b.replace("-","")))

    ufos = command_line.unidentified_options(options)
    if ufos != []:
        error_msg("Unidentified option(s) = " +  repr(ufos) + "\n" + \
                  __get_supported_command_line_option_description(options))

    if setup.input_derived_class_name != "" and \
       setup.input_derived_class_file == "":
            error_msg("Specified derived class '%s' on command line, but it was not\n" % \
                      setup.input_derived_class_name + \
                      "specified which file contains the definition of it.\n" + \
                      "use command line option '--derived-class-file'.\n")

    # check validity
    bpc = setup.bytes_per_ucs_code_point
    if bpc != "wchar_t":
        if bpc not in ["1", "2", "4"]:
            error_msg("choice for --bytes-per-ucs-code-point: %s" % bpc + \
                      "quex only supports 1, 2, or 4 bytes per character in internal engine")
            sys.exit(-1)
        else:
            setup.bytes_per_ucs_code_point = int(setup.bytes_per_ucs_code_point)

    if setup.byte_order == "<system>": 
        setup.byte_order = sys.byteorder 
    elif setup.byte_order not in ["<system>", "little", "big"]:
        error_msg("Byte order (option --endian) must be 'little' or 'big'.\n" + \
                  "Note, that this option is only interesting for cross plattform development.\n" + \
                  "By default, quex automatically chooses the endian type of your system.")

    # token offset and several ids
    if setup.input_token_counter_offset == setup.token_id_termination:
        error_msg("Token id offset (--token-offset) == token id for termination (--token-id-termination)\n")
    if setup.input_token_counter_offset == setup.token_id_uninitialized:
        error_msg("Token id offset (--token-offset) == token id for uninitialized (--token-id-uninitialized)\n")
    if setup.token_id_termination == setup.token_id_uninitialized:
        error_msg("Token id for termination (--token-id-termination) and uninitialized (--token-id-uninitialized)\n" + \
                  "are chosen to be the same. Maybe it works.", DontExitF=True)
    if setup.input_token_counter_offset < setup.token_id_uninitialized:
        error_msg("Token id offset (--token-offset) < token id uninitialized (--token-id-uninitialized).\n" + \
                  "Maybe it works.", DontExitF=True)
    if setup.input_token_counter_offset < setup.token_id_termination:
        error_msg("Token id offset (--token-offset) < token id termination (--token-id-termination).\n" + \
                  "Maybe it works.", DontExitF=True)
    
    # check that names are valid identifiers
    __check_identifier(setup, "input_token_id_prefix", "Token prefix")
    __check_identifier(setup, "output_engine_name",    "Engine name")
    if setup.input_derived_class_name != "": 
        __check_identifier(setup, "input_derived_class_name", "Derived class name")
    if setup.input_token_class_name != "": 
        __check_identifier(setup, "input_token_class_name",   "Token class name")
    
    # '--token-class' and '--token-class-file' needs to appear together
    if setup.input_token_class_name != "" and setup.input_token_class_file == "":
        error_msg("User defined token class '%s':\n" % setup.input_token_class_name + \
                  "Specifying a user-defined token class via '--token-class' requires\n" + \
                  "that the token class file, also, needs to be specified via '--token-class-file'.")
    if setup.input_token_class_file != "" and setup.input_token_class_name == "":
        error_msg("User defined token class file '%s':\n" % setup.input_token_class_file + \
                  "Specifying a user-defined token class file via '--token-class-file' requires\n" + \
                  "that the token class, also, needs to be specified via '--token-class'.")

    # __check_identifier("token_id_termination",     "Token id for termination")
    # __check_identifier("token_id_uninitialized",   "Token id for uninitialized")
    __check_file_name(setup, "input_token_class_file", "file containing user defined token class")
    __check_file_name(setup, "input_derived_class_file", "file containing user derived lexer class")

    __check_file_name(setup, "input_foreign_token_id_file", "file containing user token ids")
    __check_file_name(setup, "input_user_token_id_file",    "file containing user token ids")

    __check_file_name(setup, "input_mode_files", "quex source file")

def __check_file_name(setup, Candidate, Name):
    value = setup.__dict__[Candidate]
    CommandLineOption = SETUP_INFO[Candidate][0]

    if type(value) == list:
        for name in value:
            if name != "" and name[0] == "-": 
                error_msg("Quex refuses to work with file names that start with '-' (minus).\n"  + \
                          "Received '%s' for %s (%s)" % (value, name, repr(CommandLineOption)[1:-1]))
            if os.access(name, os.F_OK) == False:
                error_msg("File %s cannot be found.\n" % name)
    else:
        if value == "" or value[0] == "-": 
            return
        if os.access(value, os.F_OK) == False:
            error_msg("File %s cannot be found.\n" % value)

def __check_identifier(setup, Candidate, Name):
    value = setup.__dict__[Candidate]
    if is_identifier(value): return

    CommandLineOption = SETUP_INFO[Candidate][0]

    error_msg("%s must be a valid identifier (%s).\n" % (Name, repr(CommandLineOption)[1:-1]) + \
              "Received: '%s'" % value)

def __get_integer(code, option_name):
    try:
        if   type(code) == int: return code
        elif len(code) > 2:
            if   code[:2] == "0x": return int(code, 16)
            elif code[:2] == "0o": return int(code, 8)
        return int(code)
    except:
        error_msg("Cannot convert '%s' into an integer for '%s'" % (code, option_name))

def __prepare_file_name(Setup, Suffix):
    FileName = Setup.output_engine_name + Suffix
    if Setup.output_directory == "": return FileName
    else:                            return os.path.normpath(Setup.output_directory + "/" + FileName)

def __get_supported_command_line_option_description(NormalModeOptions):
    txt = "OPTIONS:\n"
    for option in NormalModeOptions:
        txt += "    " + option + "\n"

    txt += "\nOPTIONS FOR QUERY MODE:\n"
    txt += query.get_supported_command_line_option_description()
    return txt

