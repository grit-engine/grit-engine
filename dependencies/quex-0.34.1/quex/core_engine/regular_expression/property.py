from quex.input.ucs_db_parser    import ucs_property_db
from quex.frs_py.file_in         import skip_whitespace
from quex.exception              import RegularExpressionException

from quex.core_engine.regular_expression.auxiliary import __snap_until 

def do(stream):
    """Property expression: '\P{...}'
    
       Parse an expression of the forms:

       '\P{property = value}' or '\P{binary_property}'

        and return the related character set.
    """
    content = __parse_property_expression(stream, "P")
    # if len(content) < 1 or > 2 then an exception is thrown

    property_name = content[0]
    if len(content) == 1: property_value = None
    else:                 property_value = content[1]

    result = ucs_property_db.get_character_set(property_name, property_value)

    if type(result) == str:
        raise RegularExpressionException(result)

    return result

def do_shortcut(stream, ShortcutLetter, PropertyAlias):
    """Name property shortcut '\ShortcutLetter{...}' which is a shortcut
       for '\P{PropertyAlias=...}'.
    
       Parse an expression of the form '\N{CHARACTER NAME}'
       and return the related character set of characters that 
       match the given name. Wildcards in are allowed.
    """
    content = __parse_property_expression(stream, ShortcutLetter, EqualConditionPossibleF=False)
    # if len(content) != 1 then an exception is thrown

    property_value = content[0]

    result = ucs_property_db.get_character_set(PropertyAlias, property_value)

    if type(result) == str:
        raise RegularExpressionException(result)

    return result

def __parse_property_expression(stream, PropertyLetter, EqualConditionPossibleF=True):
    """Parses an expression of the form '\? { X [ = Y] }' where
       ? = PropertyLetter. If the '=' operator is present then
       two fields are returned first = left hand side, second = 
       right hand side. Othewise an element is returned.
    """
    assert len(PropertyLetter) == 1
    assert type(PropertyLetter) == str
    assert type(EqualConditionPossibleF) == bool

    # verify '\?'
    x = stream.read(2)
    if x != "\\" + PropertyLetter: 
        raise RegularExpressionException("Unicode property letter '\\%s' expected, received '%s'." % x)
    
    skip_whitespace(stream)

    x = stream.read(1)
    if x != "{": 
        raise RegularExpressionException("Unicode property '\\%s' not followed by '{'." % PropertyLetter)

    content = __snap_until(stream, "}")
    
    fields = content.split("=")

    if len(fields) == 0:
        raise RegularExpressionException("Unicode property expression '\\%s{}' cannot have no content.")

    if len(fields) > 2:
        raise RegularExpressionException("Unicode property expression '\\%s' can have at maximum one '='.")

    if not EqualConditionPossibleF and len(fields) == 2:
        raise RegularExpressionException("Unicode property expression '\\%s' does not allow '=' conditions")

    return map(lambda x: x.strip(), fields)
