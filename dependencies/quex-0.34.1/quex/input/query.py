import sys

from StringIO import StringIO

from quex.frs_py.file_in       import error_msg
from quex.core_engine.utf8     import map_unicode_to_utf8
from quex.input.ucs_db_parser  import ucs_property_db
from quex.exception            import RegularExpressionException
from quex.GetPot import GetPot

import quex.input.regular_expression as regular_expression

OPTION_DB = {
        "--property":          ["Querying properties"],
        "--set-by-property":   ["Determining character set by property"],
        "--set-by-expression": ["Determining character set by property"],
        "--property-match":    ["Find property values that match wildcards"],
        "--numeric":           ["Display sets numerically",  ["--set-by-property", "--set-by-expression"]],
        "--intervals":         ["Display sets by intervals", ["--set-by-property", "--set-by-expression"]],
}

def get_supported_command_line_option_description():
    txt = ""
    for key, description in OPTION_DB.items():
        txt += "    " + key
        if len(description) >= 2: 
            txt += " (only with "
            txt += repr(description[1])[1:-1]
            txt += ")"
        txt += "\n"
    return txt

def search_and_validate(CL, Option):

    if CL.search(Option) == False: return False

    # Validate command line
    ufos = CL.unidentified_options(OPTION_DB.keys())
    if ufos != []:
        error_msg("Unidentified option(s) = " +  repr(ufos) + "\n" + \
                  get_supported_command_line_option_description())
    return True

def do(ARGV):
    """Performs a query based on the given command line arguments.
       RETURNS: True if a query was performed.
                False if not query was requested.
    """
    cl = GetPot(ARGV)

    try:
        if   search_and_validate(cl, "--property"):          __handle_property(cl)
        elif search_and_validate(cl, "--set-by-property"):   __handle_set_by_property(cl)
        elif search_and_validate(cl, "--set-by-expression"): __handle_set_by_expression(cl)
        elif search_and_validate(cl, "--property-match"):    __handle_property_match(cl)
        else:                                                return False
        return True

    except RegularExpressionException, x:
        error_msg(x.message)

    return False

def __handle_property(cl):
    property_follower = cl.follow("", "--property")

    if property_follower == "":
        # no specific property => display all properties in the database
        sys.stderr.write("(please, wait for database parsing to complete)\n")
        ucs_property_db.init_db()
        print ucs_property_db.get_property_descriptions()

    else:
        # specific property => display information about it
        sys.stderr.write("(please, wait for database parsing to complete)\n")
        property = __get_property(property_follower)
        if property == None: return True
        print property

def __handle_property_match(cl):
    property_follower = cl.follow("", "--property-match")
    sys.stderr.write("(please, wait for database parsing to complete)\n")

    if property_follower == "":
        return

    fields = map(lambda x: x.strip(), property_follower.split("="))
    if len(fields) != 2:
        error_msg("Wrong property setting '%s'." % result)

    # -- determine name and value
    name                 = fields[0]
    wild_card_expression = fields[1]

    # -- get the property from the database
    property = __get_property(name)
    if property == None: 
        return True

    # -- find the character set for the given expression
    if property.type == "Binary":
        error_msg("Binary property '%s' is not subject to value wild card matching.\n" % property.name)

    for value in property.get_wildcard_value_matches(wild_card_expression):
        print value

def __handle_set_by_property(cl):
    result = cl.follow("", "--set-by-property") 

    # expect: 'property-name = value'
    if result != "":
        sys.stderr.write("(please, wait for database parsing to complete)\n")
        fields = map(lambda x: x.strip(), result.split("="))
        if len(fields) not in [1, 2]:
            error_msg("Wrong property setting '%s'." % result)

        # -- determine name and value
        name = fields[0]
        if len(fields) == 2: value = fields[1]
        else:                value = None

        # -- get the property from the database
        property = __get_property(name)
        if property == None: 
            return True

        # -- find the character set for the given expression
        if property.type == "Binary" and value != None:
            error_msg("Binary property '%s' cannot have a value assigned to it.\n" % property.name + \
                      "Setting ignored. Printing set of characters with the given property.")

        character_set = property.get_character_set(value)
        if character_set.__class__.__name__ != "NumberSet":
            error_msg(character_set)

        __display_set(character_set, cl)

def __handle_set_by_expression(cl):
    result = cl.follow("", "--set-by-expression")
    if result != "":
        character_set = regular_expression.parse_character_set("[:" + result + ":]")
        __display_set(character_set, cl)

def __display_set(CharSet, cl):
    if cl.search("--numeric"): display = "hex"
    else:                      display = "utf8"

    if cl.search("--intervals"): 
        print "Characters:\n", 
        __print_set_in_intervals(CharSet, display, 80)
    else:
        print "Characters:\n", 
        __print_set_single_characters(CharSet, display, 80)

    print 
   
def __get_property(Name_or_Alias):
    
    ucs_property_db.init_db()
    property = ucs_property_db[Name_or_Alias]
    if property.__class__.__name__ != "PropertyInfo":
        print property
        if Name_or_Alias.find("=") != -1: 
            print "Use command line option `--set-by-property` to investigate property settings."
        if Name_or_Alias.find("(") != -1:
            print "Use command line option `--set-by-expression` to investigate character set operations."
        return None
    
    property.init_code_point_db()
    return property

def __print_set_in_intervals(CharSet, Display, ScreenWidth):
    assert Display in ["hex", "utf8"]

    interval_list = CharSet.get_intervals(PromiseNotToChangeAnythingF=True)

    txt = ""
    line_size = 0
    for interval in interval_list:
        interval_string        = interval.get_string(Display, "-") + ", "
        interval_string_length = len(interval_string)

        if line_size + interval_string_length > ScreenWidth:
            txt += "\n"
            line_size = 0
        else:
            line_size += interval_string_length
        txt += interval_string

    print txt

def __print_set_single_characters(CharSet, Display, ScreenWidth):
    assert Display in ["hex", "utf8"]

    interval_list = CharSet.get_intervals(PromiseNotToChangeAnythingF=True)

    if Display == "hex":
        CharactersPerLine = 8
        ColumnWidth       = 6
    else:
        CharactersPerLine = 32
        ColumnWidth       = 2

    txt = ""
    line_size = 0
    character_list = []
    for interval in interval_list:
        character_list.extend(range(interval.begin, interval.end))

    # just to make sure ...
    character_list.sort()

    last_start_character_of_line = 0
    last_horizontal_offset       = 0
    for character_code in character_list:
        start_character_of_line = character_code - character_code % CharactersPerLine
        horizontal_offset       = character_code - start_character_of_line

        if start_character_of_line > last_start_character_of_line + CharactersPerLine: 
            sys.stdout.write("\n...")
        if start_character_of_line != last_start_character_of_line:
            sys.stdout.write("\n%05X: " % start_character_of_line)
            last_horizontal_offset = 0

        sys.stdout.write(" " * ColumnWidth * (horizontal_offset - last_horizontal_offset - 1))

        if Display == "hex":
            sys.stdout.write("%05X " % character_code)
        else:
            sys.stdout.write("%s " % map_unicode_to_utf8(character_code))

        last_start_character_of_line = start_character_of_line
        last_horizontal_offset       = horizontal_offset
        
    

