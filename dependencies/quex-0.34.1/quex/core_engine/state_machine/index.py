# special states:
from copy import deepcopy, copy

def clear():
    global __internal_state_index_counter
    global __map_combination_to_index
    global __map_state_machine_id_to_state_machine
    global __internal_state_machine_id_counter
    __internal_state_index_counter = long(-1)
    __map_combination_to_index = {}
    __map_state_machine_id_to_state_machine = {}    
    __internal_state_machine_id_counter = long(-1)

#     The index is chosen to be globally unique, even though, there is a constraint
#     that all target indices of a state machine must be also start indices. For connecting
#     state machines though, it is much easier to rely on a globaly unique state index.
#
#     NOTE: The index is stored in a 'long' variable. If this variable flows over, then
#           we are likely not be able to implement our state machine due to memory shortage
#           anyway.
__internal_state_index_counter = long(-1)
def get():
    """Returns a unique state index."""
    global __internal_state_index_counter
    __internal_state_index_counter += long(1)
    return __internal_state_index_counter

__map_combination_to_index = {}
def map_state_combination_to_index(cc_combination):
    """Returns index for the given combination. If the given combination
    does **not** have an index, it gets a new one. Else the existing one is
    returned."""
    cc_combination.sort()
    key_str = repr(cc_combination)

    if not __map_combination_to_index.has_key(key_str):
        # use state_machine.index.get() to get a new unique index for the combination
        __map_combination_to_index[key_str] = get()  
    
    return __map_combination_to_index[key_str]

__internal_state_machine_id_counter = long(-1)
def get_state_machine_id():
    """Produces a unique id for the state machine. This function is only to be called
       from inside the constructor of class StateMachine."""
    global __internal_state_machine_id_counter
    __internal_state_machine_id_counter += long(1)
    return __internal_state_machine_id_counter 

DELETED__map_state_machine_id_to_state_machine = {}    
def DELETED_register_state_machine(StateMachineRef):
    """Produces a unique id for the state machine. This function is only to be called
       from inside the constructor of class StateMachine."""
    assert StateMachineRef not in  __map_state_machine_id_to_state_machine.values(), \
           "error: tried to register state machine twice"
        
    global __internal_state_machine_id_counter
    __internal_state_machine_id_counter += long(1)
    # add the state machine under the give key to the database
    __map_state_machine_id_to_state_machine[__internal_state_machine_id_counter] = StateMachineRef
    return __internal_state_machine_id_counter 
    
def DELETED_get_state_machine_by_id(StateMachineID):
    assert __map_state_machine_id_to_state_machine.has_key(StateMachineID)

    return __map_state_machine_id_to_state_machine[StateMachineID]    
    
