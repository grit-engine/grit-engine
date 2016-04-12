def trim(Str):
    """Deletes whitepspace borders of a string.
       
       Transforms: input  = "  hallo, du da   "
       into        output = "hallo, du da"
    """

    if Str == "": return ""
    L = len(Str)
    for i in range(L):
        if not Str[i].isspace():
            break
    else:
        # reached end of string --> empty string
        return ""

    for k in range(1, L-i+1):
        if not Str[L-k].isspace():
            break

    # note, if k = 1 then we would return Str[i:0]
    if L-i != 1:
        if k == 1:   return Str[i:]
        else:        return Str[i:-k + 1]
    else:            return Str[i:]


def blue_print(BluePrintStr, Replacements, CommonStart="$"):
    """Takes a string acting as blue print and replaces all
       replacements of the form r in Replacements:

           r[0] = original pattern
           r[1] = replacements

       Original pattern must start with '$'.
    """
    # -- sort the replacements so that long strings
    #    are replaced first
    Replacements.sort(lambda a, b: cmp(len(b[0]), len(a[0])))

    # -- the longest original
    L = len(Replacements[0][0])

    txt      = BluePrintStr
    result   = ""
    prev_end = 0
    while 1 + 1 == 2:
        i = txt.find(CommonStart, prev_end)
        if i == -1: 
            result += txt[prev_end:]
            return result

        for orig, replacement in Replacements:
            assert orig[0] == CommonStart[0]
            if txt.find(orig, i, i + L) == i: 
                result += txt[prev_end:i] + replacement
                prev_end = i + len(orig)
                break
        else:
            # Nothing matched the expression starting with '$' simply
            # continue as if nothing happend.
            result   += txt[prev_end:i+1]
            prev_end  = i + 1
            pass



def tex_safe(Str):

    for letter in "_%&^#$":
        Str.replace(letter, "\\" + letter)

    return Str
