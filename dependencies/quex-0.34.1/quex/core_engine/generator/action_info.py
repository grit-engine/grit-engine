from quex.frs_py.file_in import is_identifier_start, \
                                is_identifier_continue

import quex.core_engine.generator.skip_code as skip_code

class ActionI:
    def __init__(self, Code, RequireTerminatingZeroF=False):
        self.__code = Code
        self.__require_terminating_zero_f = RequireTerminatingZeroF

    def set_code(self, Code):
        self.__code = Code

    def get_code(self):
        return self.__code

    def set_require_terminating_zero_f(self):
        self.__require_terminating_zero_f = True

    def require_terminating_zero_f(self):
        return self.__require_terminating_zero_f


class CodeFragment(ActionI):
    def __init__(self, Code="", RequireTerminatingZeroF=False):
        ActionI.__init__(self, Code, RequireTerminatingZeroF)


UserCodeFragment_OpenLinePragma = {
#___________________________________________________________________________________
# Line pragmas allow to direct the 'virtual position' of a program in a file
# that was generated to its origin. That is, if an error occurs in line in a 
# C-program which is actually is the pasted code from a certain line in a .qx 
# file, then the compiler would print out the line number from the .qx file.
# 
# This mechanism relies on line pragmas of the form '#line xx "filename"' telling
# the compiler to count the lines from xx and consider them as being from filename.
#
# During code generation, the pasted code segments need to be followed by line
# pragmas resetting the original C-file as the origin, so that errors that occur
# in the 'real' code are not considered as coming from the .qx files.
# Therefore, the code generator introduces placeholders that are to be replaced
# once the whole code is generated.
#
#  ...[Language][0]   = the placeholder until the whole code is generated
#  ...[Language][1]   = template containing 'NUMBER' and 'FILENAME' that
#                       are to replaced in order to get the resetting line pragma.
#___________________________________________________________________________________
        "C": [
              '/* POST-ADAPTION: FILL IN APPROPRIATE LINE PRAGMA */',
              '#line NUMBER "FILENAME"'
             ],
   }

class UserCodeFragment(ActionI):
    def __init__(self, Code, Filename, LineN, LanguageDB=None, AddReferenceCodeF=False):
        assert type(Code)       in [str, unicode]
        assert type(LanguageDB) == dict or LanguageDB == None
        assert type(Filename)   in [str, unicode]
        assert type(LineN)      in [int, long, float]

        self.filename = Filename
        self.line_n   = LineN

        # No clue yet, how to deal with languages other than C/C++ here.
        if AddReferenceCodeF and Code != "":
            txt  = '\n#line %i "%s"\n' % (self.line_n, self.filename)
            txt += Code
            if txt[-1] != "\n": txt = txt + "\n"
            txt += UserCodeFragment_OpenLinePragma["C"][0] + "\n"
            code = txt
        else:
            code = Code

        require_terminating_zero_f = False
        if LanguageDB != None and LanguageDB["$require-terminating-zero-preparation"](LanguageDB, code):
            require_terminating_zero_f = True

        ActionI.__init__(self, code, require_terminating_zero_f)


def UserCodeFragment_straighten_open_line_pragmas(filename, Language):
    if Language not in UserCodeFragment_OpenLinePragma.keys():
        return

    try:    fh = open(filename)
    except: raise "error: file to straighten line pragmas not found: '%s'" % \
            filename

    new_content = ""
    line_n      = 0
    LinePragmaInfo = UserCodeFragment_OpenLinePragma[Language]
    for line in fh.readlines():
        line_n += 1
        if line.find(LinePragmaInfo[0]) != -1:
            if Language == "C":
                line = LinePragmaInfo[1]
                line = line.replace("NUMBER", repr(int(line_n)))
                line = line.replace("FILENAME", filename)
                line = line + "\n"
        new_content += line

    fh.close()

    fh = open(filename, "w")
    fh.write(new_content)
    fh.close()

class PatternActionInfo:
    def __init__(self, PatternStateMachine, Action, Pattern="", IL = None, ModeName=""):

        assert Action == None or \
               issubclass(Action.__class__, ActionI) or \
               type(Action) in [str, unicode]
        assert PatternStateMachine.__class__.__name__ == "StateMachine" \
               or PatternStateMachine == None

        self.__pattern_state_machine = PatternStateMachine
        if type(Action) in [str, unicode]: self.__action = CodeFragment(Action)
        else:                              self.__action = Action

        self.pattern               = Pattern
        # depth of inheritance where the pattern occurs
        self.inheritance_level     = IL
        self.inheritance_mode_name = ModeName

    def pattern_state_machine(self):
        return self.__pattern_state_machine

    def action(self):
        return self.__action

    def pattern_index(self):
        return self.pattern_state_machine().get_id()

    def __repr__(self):         
        txt  = ""
        txt += "self.pattern           = " + repr(self.pattern) + "\n"
        txt += "self.pattern_state_machine = \n" + repr(self.pattern_state_machine()).replace("\n", "\n      ")
        txt += "self.action            = " + repr(self.action().get_code()) + "\n"
        if self.action().__class__ == UserCodeFragment:
            txt += "self.filename          = " + repr(self.action().filename) + "\n"
            txt += "self.line_n            = " + repr(self.action().line_n) + "\n"
        txt += "self.inheritance_level = " + repr(self.inheritance_level) + "\n"
        txt += "self.pattern_index     = " + repr(self.pattern_state_machine().core().id()) + "\n"
        return txt

