from copy import copy
import StringIO


def map_utf8_to_unicode(utf8_string_or_stream):
    """Maps the start of the 'utf8_string' to a single unicode character. 
    
       Returns: (string) the unicode of the letter and the remaining utf8-string with the first 
                         letters cut that represented the unicode letter.
                (StringIO) the unicode of the first utf8 coding in the stream. sets the stream
                           position to the first character after the utf8 code.         
    """    
    arg_type=utf8_string_or_stream.__class__.__name__
    assert arg_type in ["StringIO", "file", "str"]

    if    utf8_string_or_stream.__class__.__name__ == "StringIO" \
       or utf8_string_or_stream.__class__.__name__ == "file":
       return __read_one_utf8_code_from_stream(utf8_string_or_stream)
    elif utf8_string_or_stream.__class__.__name__ == "str":
       utf8_string = utf8_string_or_stream

    stream = StringIO.StringIO(utf8_string)
    ucs_character_code = __read_one_utf8_code_from_stream(stream)
    remainder = stream.read()  
    return ucs_character_code, remainder 
    
def map_n_utf8_to_unicode(utf8_string, N=-1, RemainderF=False):
    """Reads 'N' characters out of the string and creates an array
       of integers that represent the unicode characters contained.
       N=-1 => interpret the whole given string.

       Returns: unicode value and if RemainderF=True: remaining string.
    """   
    stream = StringIO.StringIO(utf8_string)
    result = []
    if N == 0: 
       if RemainderF: return result, utf8_string
       else:          return result

    if N != -1:
        tmp = __read_one_utf8_code_from_stream(stream)
        result.append(tmp)
        n = 0
        while tmp != 0xFF:
            n += 1
            if n >= N: break
            tmp = __read_one_utf8_code_from_stream(stream)
            result.append(tmp)

    else:
        tmp = __read_one_utf8_code_from_stream(stream)
        while tmp != 0xFF:
            result.append(tmp)
            tmp = __read_one_utf8_code_from_stream(stream)

    if RemainderF: 
        remainder = stream.read()    
        return result, remainder
    else:          
        return result

def map_unicode_to_utf8(UCS_character_code):
    """Returns a utf8 string that correspond to the unicode character
       passed by UCS_character_code.
    """
    
    if UCS_character_code < 0x80:
        return chr(UCS_character_code)
    elif UCS_character_code <      0x800:    # anyway: UCS_character_code >= 0x80
        byte_n = 1
    elif UCS_character_code <    0x10000:    # anyway: UCS_character_code >= 0x800
        byte_n = 2
    elif UCS_character_code <   0x200000:    # anyway: UCS_character_code >= 0x10000 
        byte_n = 3
    elif UCS_character_code <  0x4000000:    # anyway: UCS_character_code >= 0x200000
        byte_n = 4
    elif UCS_character_code < 0x10000000:    # anyway: UCS_character_code >= 0x4000000
        byte_n = 5
    else:
        return [ -1 ]

    # as utf-8 specifies: number of ones at head + 1 = number of trailing bytes
    leading_ones_str      = "1" * (byte_n + 1) + "0"
    char_bits_in_header_n = 8 - (byte_n + 1 + 1)
    
    # number of bits reserved for the character:
    #  remaining bits in the header byte = 8 - (byte_n + 1)
    #                                      (+1 because of the trailing zero)
    #  bits per byte that follows        = 6
    bit_n = char_bits_in_header_n + 6 * byte_n 

    # get bit representation of the original UCS character code
    orig_bit_str = __int_to_bit_string(UCS_character_code, bit_n)
    bytes = [0] * (byte_n + 1)
    bytes[0] = __bit_string_to_int(leading_ones_str
                                   + orig_bit_str[:char_bits_in_header_n])

    for i in range((bit_n - char_bits_in_header_n)/6):
        bit_0 = char_bits_in_header_n + i * 6
        bytes[i+1] = __bit_string_to_int("10" + orig_bit_str[bit_0:bit_0+6])

    result_str = ""
    for byte in bytes:
       result_str += chr(byte)    
    return result_str

def __int_to_bit_string(IntN, BitN):
    """Receives an integer and constructs a string containing 0s and 1s that 
       represent the integer binarily.
    """
    int_n = copy(IntN)

    sum = ""
    while int_n:
        if int_n & 0x1: sum = "1" + sum
        else:           sum = "0" + sum
        int_n = int_n >> 1
    
    # NOTE: this should not occur in our case (head character > 0xC0)
    if len(sum) < BitN: sum = "0" * (BitN - len(sum)) + sum
    
    return sum

def __bit_string_to_int(BitStr):
    """Transforms a string of the form '01001001' into an integer
    which represents this byte.

    TESTED <frs>
    """

    BitStr = BitStr.replace(".", "")
    BitArray = map(lambda x: x, BitStr)
    BitArray.reverse()

    n = 0
    sum = 0
    for bit in BitArray:
        if bit == "1": sum += 2**n
        n += 1

    return sum

def __read_one_utf8_code_from_stream(char_stream):
    """Interprets the subsequent characters as a UTF-8 coded
    character.

    RETURNS: integer value of the unicode character.
             0xFF          in case of error.

             second parameter: True if character was backslashed,
                               False if not.
    """
    character = char_stream.read(1)
    if character == "": return 0xFF
    try:    head_char = ord(character)
    except: return 0xFF

    # (*) head characters for the the "utf-8 escape" are located in between
    #     0xC0 to 0xFD. If the chacter code falls out of this border no further
    #     consideration is required.
    if head_char < 0xC0 or head_char > 0xFD:
        return head_char


    # (*) determine byte range and number of characters
    #     that have to be decoded.
    head = __int_to_bit_string(head_char, 8)

    if   head[:3] == "110":
        char_range = [0x80,      0x7FF]
        char_n     = 1
    elif head[:4] == "1110":
        char_range = [0x800,     0xFFFF]
        char_n     = 2
    elif head[:5] == "11110":
        char_range = [0x10000,   0x1FFFFF]
        char_n     = 3
    elif head[:6] == "111110":
        char_range = [0x200000,  0x3FFFFFF]
        char_n     = 4
    elif head[:7] == "1111110":
        char_range = [0x4000000, 0x7FFFFFFF]
        char_n     = 5
    else:
        print "utf8.py: wrong utf-8 header byte %s" %  header
        return 0xFF
    
    # (*) read the bytes from the char_stream that are needed
    try:    bytes = map(lambda x: ord(x), char_stream.read(char_n))
    except:
        print "utf8.py: could not read %i utf-follow bytes" % char_n
        return 0xFF
    
    # -- take the remaining bits from the header
    bits = head[char_n+2:]
    # -- take from each following byte the 6 lowest bits 
    for byte in bytes:
        # utf-8 says that any follower byte has to be in range [0x80-0xBF]
        if byte < 0x80 or byte > 0xBF: 
            print "utf8.py: follow-byte out of range: %02x not in [0x80-0xBF]" % byte
            return 0xFF
        
        byte_str = __int_to_bit_string(byte, 8)
        # consider only the 6 lowest bits
        bits += byte_str[2:]

    return __bit_string_to_int(bits)


 

