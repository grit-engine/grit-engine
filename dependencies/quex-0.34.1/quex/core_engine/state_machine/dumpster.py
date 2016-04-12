
class CachedValue:
    def __init__(self):
        self.__value = None
        self.__valid_f = False

    def __call__(self):
        return __value

    def set(self, Value):
        self.__value   = Value
        self.__valid_f = True

    def invalid(self):
        self.__valid_f = False

    def is_valid():
        return self.__valid_f


class TransitionMapValueCache:
    def __init__(self):
        self.trigger_set_union = CachedValue()
        self.trigger_map       = CachedValue()
        self.is_dfa_f          = CachedValue()

    def invalid(self):
        self.trigger_set_union.invalid()
        self.trigger_map.invalid()
        self.is_dfa_f.invalid()
