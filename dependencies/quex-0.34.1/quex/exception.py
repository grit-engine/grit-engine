class RegularExpressionException(Exception):
    def __init__(self, InfoStr):
        self.message = InfoStr

    def __repr__(self):
        txt = "QuexException:\n"
        return txt + self.message

