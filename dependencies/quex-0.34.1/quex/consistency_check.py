import sys

from   quex.frs_py.file_in import error_msg
import quex.lexer_mode     as     lexer_mode
from   quex.core_engine.generator.action_info import CodeFragment


def do(Modes):
    """If consistency check fails due to a fatal error, then this functions
    exits back to the system with error code -1.  Otherwise, in 'not so
    perfect' cases there might be only a warning being printed to standard
    output.
    """
    if len(Modes) == 0:
        error_msg("No single mode defined - bailing out", Prefix="consistency check")

    # -- is there a mode that is applicable?
    for mode in Modes.values():
        if mode.options["inheritable"] != "only": break
    else:
        error_msg("There is no mode that can be applied---all existing modes are 'inheritable only'.\n" + \
                "modes are = " + repr(map(lambda m: m.name, Modes.values()))[1:-1],
                  Prefix="consistency check")

    # -- is the initial mode defined
    if lexer_mode.initial_mode.get_code() == "":
        # find first mode that can actually be applied
        for mode in Modes.values():
            if mode.options["inheritable"] != "only":
                selected_mode = mode.name
                break
            
        lexer_mode.initial_mode = CodeFragment(selected_mode)
        error_msg("no initial mode defined via 'start'\n" + \
                  "using mode '%s' as initial mode" % selected_mode, DontExitF=True,
                  Prefix="warning")


    # -- is the start mode applicable?
    if Modes.has_key(lexer_mode.initial_mode.get_code()) == False:
        error_msg("Start mode '%s' has not been defined anywhere." % lexer_mode.initial_mode.get_code(),
                  lexer_mode.initial_mode.filename, lexer_mode.initial_mode.line_n)

    if Modes[lexer_mode.initial_mode.get_code()].options["inheritable"] == "only":
        error_msg("Start mode '%s' is inheritable only and cannot be instantiated." % lexer_mode.initial_mode.get_code(),
                  lexer_mode.initial_mode.filename, lexer_mode.initial_mode.line_n)

    # -- check for circular inheritance
    check_circular_inheritance(Modes)

    # -- mode specific checks
    for mode in Modes.values():
        mode.consistency_check()

def check_circular_inheritance(ModeDB):
    for mode in ModeDB.values():
        __search_circular_inheritance(ModeDB, mode, [])
        mode.inheritance_circularity_check_done_f = True

def __search_circular_inheritance(ModeDB, mode, inheritance_path):
    
    def __report_error(mode, inheritance_path):
        assert inheritance_path != []
        msg = ""
        previous = mode.name
        inheritance_path.reverse()
        for mode_name in inheritance_path:
            msg += "mode '%s' is a base of mode '%s'.\n" % (previous, mode_name)
            previous = mode_name

        error_msg("circular inheritance detected:\n" + msg, mode.filename, mode.line_n)

    for base_mode_name in mode.base_modes:
        # -- does base mode exist?
        if ModeDB.has_key(base_mode_name) == False:
            error_msg("mode '%s' is derived from mode a '%s'\n" % (mode.name, base_mode_name) + \
                      "but no such mode '%s' actually exists!" % base_mode_name,
                      mode.filename, mode.line_n)

        # -- did mode appear in the path of deriving modes
        base_mode = ModeDB[base_mode_name]
        if base_mode.name in inheritance_path: __report_error(base_mode, inheritance_path)

        __search_circular_inheritance(ModeDB, base_mode, inheritance_path + [mode.name])

